#pragma once
/*
Client Module
*/
#include <stdlib.h>
#include "../../ze_common/ze_common_full.h"

#include "client_render.h"
#include "client_internal.h"

/////////////////////////////////////////////////////////////
// Configures this client's time-base relative
// to the server.
// Client will be ((ping / 2) + jitter delay) behind
// the server
/////////////////////////////////////////////////////////////
internal void CL_SetServerTick(i32 value)
{
    i32 result = value - APP_DEFAULT_JITTER_TICKS;
	//g_serverTick = result;
    g_sim.tick = result;
    g_bHasSimSynced = YES;
    APP_LOG(64, "CL Set Sim Tick base at %d\n", result);
}

internal i32 CL_HasSimSynced()
{
    return g_bHasSimSynced;
}

// Can be changed during command execute so always retreive from here:
internal i32 CL_GetServerTick()
{
	//return g_serverTick;
    return g_sim.tick;
}

////////////////////////////////
// Implementation
////////////////////////////////
#include "client_input.h"
#include "client_game.h"
#include "../shared/commands_serialise.h"
#include "../shared/commands_deserialise.h"
#include "client_user_sync.h"
#include "client_packets.h"
#include "client_connect.h"
#include "client_run_commands.h"

#include "client_debug.h"

extern "C" i32 CL_IsRunning() { return g_isRunning; }

//////////////////////////////////////////
// Write Render data
//////////////////////////////////////////
extern "C" void CL_WriteDrawFrame(ZEByteBuffer* list, ZEByteBuffer* data)
{
    f64 startTime = App_SampleClock();
    Transform* camera = &g_camera;
	if (g_clDebugFlags & CL_DEBUG_FLAG_DEBUG_CAMERA)
	{
		camera = &g_debugCamera;
	}
	else
	{
		SimEntity* plyr = Sim_GetEntityBySerial(&g_sim, g_avatarSerial);
    	if (plyr != NULL)
    	{
    	    camera = &plyr->body.t;
    	}
	}
	
    #if 0
    M3x3_SetToIdentity(g_camera.rotation.cells);
    M3x3_RotateY(g_camera.rotation.cells, g_testCameraDegrees.y * DEG2RAD);
    M3x3_RotateX(g_camera.rotation.cells, g_testCameraDegrees.x * DEG2RAD);
    #endif
    g_rendCfg.debugFlags = g_clDebugFlags;
    // add extra bit
    if (g_bVerboseFrame == YES)
    {
        printf("CL - mark verbose frame\n");
        g_rendCfg.debugFlags |= CL_DEBUG_FLAG_VERBOSE_FRAME;
        g_bVerboseFrame = NO;
    }
    ZRViewFrame* frame = CLR_WriteDrawFrame(
        list, data, &g_sim, camera, g_debugObjs, g_numDebugObjs, g_rendCfg);
    f64 endTime = App_SampleClock();
    frame->prebuildTime = endTime - startTime;
}

internal void* CL_Malloc(i32 numBytes)
{
    ZE_ASSERT(g_numAllocations < CL_MAX_ALLOCATIONS,
        "No space to record malloc")
    i32 index = g_numAllocations++;
    g_allocations[index] = malloc(numBytes);
    g_bytesAllocated += numBytes;
    return g_allocations[index];
}

extern "C" u8 CL_ParseCommandString(char* str, char** tokens, i32 numTokens)
{
    return 0;
}


//////////////////////////////////////////
// Load Scene
//////////////////////////////////////////
void CL_LoadTestScene()
{
	Sim_LoadStaticScene(&g_sim, 0);
	//g_sim.boundaryMin = { -6, -6, -6 };
    //g_sim.boundaryMax = { 6, 6, 6 };
	
	// Add local test avatar
	#if 0
	SimEntSpawnData def = {};
    def = {};
    def.isLocal = 1;
	g_avatarSerial = Sim_ReserveEntitySerial(&g_sim, def.isLocal);
    def.serial = g_avatarSerial;
	def.tickType = SIM_ENT_TYPE_ACTOR;
    def.pos[1] = 0;
    def.scale[0] = 1;
    def.scale[1] = 1;
    def.scale[2] = 1;
    Sim_RestoreEntity(&g_sim, &def);
	#endif

    #if 1 // Create target sight
    
    SimEntSpawnData def = {};
    def.isLocal = 1;
    def.serial = Sim_ReserveEntitySerial(&g_sim, def.isLocal);
    def.factoryType = SIM_FACTORY_TYPE_TARGET_POINT;
    g_userTargetSerial = def.serial;
    Sim_RestoreEntity(&g_sim, &def);
    #endif
    APP_PRINT(64, "CL test scene initialised\n")
}

// Public so that local user can be instantly set from outside
extern "C" void CL_SetLocalUser(UserIds ids)
{
    ZE_ASSERT(g_clientState == CLIENT_STATE_REQUESTING,
        "Client is not requesting a connection")
    APP_LOG(64, "CL Set local user public %d private %d\n",
        ids.publicId, ids.privateId
    );
    g_ids = ids;
    //g_clientState = CLIENT_STATE_SYNC;
    g_clientState = CLIENT_STATE_PLAY;
}

////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////
extern "C" void CL_Init()
{
    APP_PRINT(32, "CL - Init\n");
}


//////////////////////////////////////////
// Start Session
//////////////////////////////////////////
extern "C" void CL_Start(ZNetAddress serverAddress, i32 updSocketId)
{
    ZE_ASSERT(g_clientState == CLIENT_STATE_NONE,
        "Client State is not clear")
    APP_PRINT(32, "CL Init scene\n");

    g_udpSocketId = updSocketId;
    g_serverAddress = serverAddress;
	g_clientState = CLIENT_STATE_REQUESTING;
    i32 cmdBufferSize = MegaBytes(1);

    g_isRunning = YES;
    //ZEByteBuffer a = Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize);
    //ZEByteBuffer b = Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize);

    g_rendCfg.worldLightsMax = 100;
    g_rendCfg.extraLightsMax = 100;

    i32 maxEnts = APP_MAX_ENTITIES;
    i32 numEntityBytes = Sim_CalcEntityArrayBytes(maxEnts);
    SimEntity* mem = (SimEntity*)CL_Malloc(numEntityBytes);
    Sim_Init("Client", &g_sim, mem, maxEnts);
	Sim_Reset(&g_sim);
    CL_LoadTestScene();

    APP_PRINT(128, "CL Init net stream buffers\n");
    COM_InitStream(&g_reliableStream,
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize),
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize)
    );
    COM_InitStream(&g_unreliableStream,
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize),
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize)
    );

    Transform_SetToIdentity(&g_camera);
    g_camera.pos.z = 10;
    g_camera.pos.y += 34;
    Transform_SetRotation(&g_camera, -(80.0f    * DEG2RAD), 0, 0);

    g_testCameraDegrees.x = -80.0f * DEG2RAD;

    CLR_Init(App_GetAssetDB());

    CLDebug_Init();
    /*
    i32 numRenderCommandBytes = sizeof(RenderCommand) *
		CL_MAX_RENDER_COMMANDS;
    u8* bytes = (u8*)CL_Malloc(numRenderCommandBytes);
    ZE_SET_ZERO(bytes, numRenderCommandBytes);

    g_renderCommands = (RenderCommand*)bytes;
    */
    APP_PRINT(64, "CL Init inputs\n");
    CL_InitInputs(&g_inputActions);

    APP_PRINT(64, "CL init completed with %d allocations (%dKB)\n ",
        g_numAllocations, (u32)BytesAsKB(g_bytesAllocated));
}


//////////////////////////////////////////
// Shutdown
//////////////////////////////////////////
extern "C" void CL_Shutdown()
{
    g_isRunning = NO;
    CLR_Shutdown();
	for (i32 i = 0; i < g_numAllocations; ++i)
	{
		free(g_allocations[i]);
	}
	g_numAllocations = 0;
}

internal void CL_CalcPings(timeFloat deltaTime)
{
	g_ping = Ack_CalculateAverageDelay(&g_acks);
	g_jitter = (g_acks.delayMax - g_acks.delayMin);
}

//////////////////////////////////////////
// Tick Game
//////////////////////////////////////////
internal void CL_TickInGame(timeFloat deltaTime, i64 platformFrame)
{
    S2C_InputResponse* latestResponse =
        CLI_FindLatestInputResponse(g_serverResponses, CL_GetServerTick());
    if (latestResponse != NULL)
    {
        APP_LOG(96, "CLI Latest historical server response is SV tick %d\n",
            latestResponse->header.tick);
        C2S_Input* matchingInput = CL_RecallSentInputCommandByServerTick
            (g_sentCommands, latestResponse->header.tick);
		if (matchingInput != NULL)
		{
			APP_LOG(256, "CL tick record %.3f, %.3f, %.3f vs sv response %.3f, %.3f, %.3f\n",
				matchingInput->avatarPos.x,
				matchingInput->avatarPos.y,
				matchingInput->avatarPos.z,
				latestResponse->latestAvatarPos.x,
				latestResponse->latestAvatarPos.y,
				latestResponse->latestAvatarPos.z
			);
		}
    }

    if (g_bHasNewResponse == YES)
    {
        g_bHasNewResponse = NO;
        CLG_SyncAvatar(
            &g_sim,
            g_avatarSerial,
            &g_lastInputResponse,
            g_sentCommands,
            g_userInputSequence,
            g_ping,
            g_elapsed);
    }

    CL_ProcessDebugInput(&g_inputActions, platformFrame);
    
    // Until Sim sync has begun, input is ignored!
    C2S_Input cmd = {};
    if (CL_HasSimSynced() == YES)
    {
        // Update input
		if (CLDebug_IsDebugInputActive() == NO)
		{
			CL_UpdateActorInput(&g_inputActions, &g_actorInput);
			g_testCameraDegrees = g_actorInput.degrees;
		}
		else
		{
			// Debugging - Clear input to server
			g_actorInput.buttons = 0;
			g_actorInput.prevButtons = 0;
		}
		
        
	    // Create and store input to server
	    SimEntity* plyr = Sim_GetEntityBySerial(&g_sim, g_avatarSerial);
	    Vec3 pos = {};
	    if (plyr)
	    {
	    	plyr->input = g_actorInput;
	    	pos = plyr->body.t.pos;
	    }
	    else
	    {
	    	APP_LOG(64, "No player!\n");
	    }
	    Cmd_InitClientInput(
            &cmd,
            g_userInputSequence++,
            &g_actorInput,
            &pos,
            CL_GetServerTick(),
            g_elapsed,
            deltaTime);
	    CL_StoreSentInputCommand(g_sentCommands, &cmd);
    }
	// Run
    CLG_TickGame(&g_sim, deltaTime);
    g_elapsed += deltaTime;

    if (CL_HasSimSynced() == YES)
    {
        CL_WriteAndSendPacketFromStreams(&g_sim.quantise, g_elapsed, &cmd);
    }
    else
    {
        CL_WriteAndSendPacketFromStreams(&g_sim.quantise, g_elapsed, NULL);
    }
}


//////////////////////////////////////////
// Tick
//////////////////////////////////////////
void CL_Tick(ZEByteBuffer* sysEvents, timeFloat deltaTime, i64 platformFrame)
{
    // APP_PRINT(128, "*** CL SIM TICK %d (Input Seq %d, T %.3f) ***\n",
    //     CL_GetServerTick(), g_userInputSequence, g_elapsed);
    APP_LOG(128, "*** CL SIM TICK %d (Input Seq %d, T %.3f) ***\n",
        CL_GetServerTick(), g_userInputSequence, g_elapsed);
    APP_LOG(128, "\tLatest input ack before packet read: %d\n", g_latestUserInputAck);
    CL_ReadSystemEvents(sysEvents, deltaTime, platformFrame);

    CL_CalcPings(deltaTime);
    APP_LOG(128, "CL Measured Ping %.5f, Jitter %.5f\n", g_ping, g_jitter);
    
	CL_RunReliableCommands(&g_sim, &g_reliableStream, deltaTime);
    //CL_LogCommandBuffer(&g_unreliableStream.inputBuffer, "Unreliable input");
    CL_RunUnreliableCommands(&g_sim, &g_unreliableStream, deltaTime);

    if (g_clientState == CLIENT_STATE_PLAY)
    {
        CL_TickInGame(deltaTime, platformFrame);
    }
    else if (g_clientState == CLIENT_STATE_REQUESTING)
    {
        CL_TickRequesting(deltaTime, platformFrame);
    }
    else if (g_clientState == CLIENT_STATE_SYNC)
    {

    }
    else
    {
        printf("CL - bad state\n");
    }
    
    CLDebug_UpdateDebugObjects(deltaTime);
}

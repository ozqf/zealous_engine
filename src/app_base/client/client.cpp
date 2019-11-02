#pragma once

#include <stdlib.h>
#include "../../ze_common/ze_common_full.h"
#include "client.h"
#include "../../sys_events.h"
#include "../../sim/sim.h"
#include "client_input.h"

#define CLIENT_STATE_NONE 0
#define CLIENT_STATE_REQUESTING 1
#define CLIENT_STATE_HANDSHAKE 2
#define CLIENT_STATE_SYNC 3
#define CLIENT_STATE_PLAY 4

internal i32 g_clientState = CLIENT_STATE_NONE;

internal i32 g_isRunning = 0;
internal SimScene g_sim;
internal i32 g_ticks = 0;
internal timeFloat g_elapsed = 0;
internal i32 g_serverTick = 0;
internal timeFloat g_ping;
internal timeFloat g_jitter;

internal i32 g_avatarSerial = 0;

#define CL_MAX_ALLOCATIONS 256
internal void* g_allocations[CL_MAX_ALLOCATIONS];
internal i32 g_bytesAllocated = 0;
internal i32 g_numAllocations = 0;

internal NetStream g_reliableStream;
internal NetStream g_unreliableStream;
internal UserIds g_ids;
internal AckStream g_acks;
internal ZNetAddress g_serverAddress;
internal i32 g_userInputSequence = 0;
internal i32 g_latestUserInputAck = 0;
internal Vec3 g_latestAvatarPos = {};

internal Vec3 g_testHitPos = { 0, 2, 0 };
internal M4x4 g_matrix;

internal i32 g_interpolateRenderScene = 0;
//internal i32 g_tickEnemies = 1;

#define CL_DEBUG_FLAG_NO_ENEMY_TICK (1 << 0)
internal i32 g_clDebugFlags = 0
    //| CL_DEBUG_FLAG_NO_ENEMY_TICK
;

// Menus
internal i32 g_mainMenuOn;

#define CL_MAX_INPUT_ACTIONS 256
internal InputAction g_inputActionItems[CL_MAX_INPUT_ACTIONS];
internal InputActionSet g_inputActions = {
    g_inputActionItems,
    0
};

// Buffer transmitted inputs
internal C2S_Input g_sentCommands[CL_MAX_SENT_INPUT_COMMANDS];

//#define CL_MAX_RENDER_COMMANDS 1024
//internal RenderCommand* g_renderCommands;

internal SimActorInput g_actorInput = {};

#include "client_render.h"
#include "client_game.h"
#include "../commands_serialise.h"
#include "../commands_deserialise.h"
#include "client_packets.h"

i32 CL_IsRunning() { return g_isRunning; }

internal void CL_WriteNetworkDebug(CharBuffer* str)
{
	//char* chars = str->chars;
	//i32 written = 0;
    str->cursor += sprintf_s(
        str->cursor,
        str->Space(),
        "CLIENT:\nServer Tick: %d\nTick: %d\nElapsed: %.3f\nOutput Seq: %d\nAck Seq: %d\nDelay: %.3f\nJitter %.3f\n",
        g_serverTick, g_ticks, g_elapsed, g_acks.outputSequence,
		g_acks.remoteSequence, g_ping, g_jitter
    );


    str->cursor += sprintf_s(
            str->cursor,
            str->Space(),
			"=== Commands ===\n%d reliablebytes %d\n%d unreliable bytes %d\n",
            Stream_CountCommands(&g_reliableStream.inputBuffer).count,
            g_reliableStream.inputBuffer.Written(),
            Stream_CountCommands(&g_unreliableStream.inputBuffer).count,
            g_unreliableStream.inputBuffer.Written()
            );

    #if 0
    SimEntity* ent =  Sim_GetEntityBySerial(&g_sim, -1);
    if (ent)
    {
        written += sprintf_s(chars + written, str->maxLength,
			"World vol pos Y: %.3f\n", ent->t.pos.y);
    }
    #endif
	#if 0
	// currently overflows debug text buffer:
	for (i32 i = 0; i < ACK_CAPACITY; ++i)
	{
		AckRecord* rec = &g_acks.awaitingAck[i];
		if (rec->acked)
		{
			timeFloat time = rec->receivedTime - rec->sentTime;
			written += sprintf_s(chars + written, str->maxLength,
				"%.3f Sent: %.3f Rec: %.3f\n",
				time, rec->sentTime, rec->receivedTime
            );
		}
	}
	#endif
	//str->length = written;
}

internal void CL_WriteTransformDebug(CharBuffer* str)
{
	char* chars = str->chars;
	i32 written = 0;
    f32* m = g_matrix.cells;
    written += sprintf_s(chars, str->maxLength,
        "MATRIX:\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n",
        m[0], m[4], m[8], m[12],
        m[1], m[5], m[9], m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15]
    );
}

internal void CL_WriteCameraDebug(CharBuffer* str)
{
	
}

void CL_WriteDebugString(CharBuffer* str)
{
	CL_WriteNetworkDebug(str);
	//CL_WriteTransformDebug(str);
	//CL_WriteCameraDebug(str);
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

u8 CL_ParseCommandString(char* str, char** tokens, i32 numTokens)
{
    return 0;
}

void CL_LoadTestScene()
{
	Sim_LoadScene(&g_sim, 0);
	
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
}

// Public so that local user can be instantly set from outside
void CL_SetLocalUser(UserIds ids)
{
    ZE_ASSERT(g_clientState == CLIENT_STATE_REQUESTING,
        "Client is not requesting a connection")
    APP_LOG(64, "CL Set local user public %d private %d\n",
        ids.publicId, ids.privateId
    );
    g_ids = ids;
    g_clientState = CLIENT_STATE_SYNC;
}

////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////
void CL_Init(ZNetAddress serverAddress)
{
    ZE_ASSERT(g_clientState == CLIENT_STATE_NONE,
        "Client State is not clear")
    APP_PRINT(32, "CL Init scene\n");
    g_serverAddress = serverAddress;
	g_clientState = CLIENT_STATE_REQUESTING;
    i32 cmdBufferSize = MegaBytes(1);
    //ZEByteBuffer a = Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize);
    //ZEByteBuffer b = Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize);

    i32 maxEnts = APP_MAX_ENTITIES;
    i32 numEntityBytes = Sim_CalcEntityArrayBytes(maxEnts);
    SimEntity* mem = (SimEntity*)CL_Malloc(numEntityBytes);
    Sim_Init("Client", &g_sim, mem, maxEnts);
	Sim_Reset(&g_sim);
    CL_LoadTestScene();

    COM_InitStream(&g_reliableStream,
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize),
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize)
    );
    COM_InitStream(&g_unreliableStream,
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize),
        Buf_FromMalloc(CL_Malloc(cmdBufferSize), cmdBufferSize)
    );

    /*
    i32 numRenderCommandBytes = sizeof(RenderCommand) *
		CL_MAX_RENDER_COMMANDS;
    u8* bytes = (u8*)CL_Malloc(numRenderCommandBytes);
    ZE_SET_ZERO(bytes, numRenderCommandBytes);

    g_renderCommands = (RenderCommand*)bytes;
    */

    CL_InitInputs(&g_inputActions);

    APP_LOG(64, "CL init completed with %d allocations (%dKB)\n ",
        g_numAllocations, (u32)BytesAsKB(g_bytesAllocated));
}

void CL_Shutdown()
{
	for (i32 i = 0; i < g_numAllocations; ++i)
	{
		free(g_allocations[i]);
	}
	g_numAllocations = 0;
}
#if 0
internal void CL_ReadReliableCommands(NetStream* stream)
{
    ZEByteBuffer* b = &stream->inputBuffer;
    u8* read = b->start;
    u8* end = b->ptrEnd;
    while (read < end)
    {
        Command* header = (Command*)read;
        ZE_ASSERT(header->sentinel == CMD_SENTINEL)
        ZE_ASSERT(header->size > 0)
        read += header->size;
        switch (header->type)
        {
            case CMD_TYPE_IMPULSE:
            {
                CmdImpulse* cmd = (CmdImpulse*)header;
            } break;
            default:
            {
                printf("CL Unknown command type %d\n", header->type);
                ILLEGAL_CODE_PATH
            } break;
        }
    }
}
#endif
internal void CL_ReadSystemEvents(
	ZEByteBuffer* sysEvents, timeFloat deltaTime, frameInt platformFrame)
{
    AppTimer timer(APP_STAT_CL_INPUT, g_sim.tick);
    //printf("CL Reading platform events (%d bytes)\n", sysEvents->Written());
    u8* read = sysEvents->start;
    u8* end = sysEvents->cursor;
    while (read < end)
    {
        SysEvent* ev = (SysEvent*)read;
        i32 err = Sys_ValidateEvent(ev);
        if (err != ZE_ERROR_NONE)
        {
            printf("CL Error %d reading system event header\n", err);
            return;
        }
        read += ev->size;
        switch(ev->type)
        {
            case SYS_EVENT_PACKET:
            {
				//COM_PrintBytes((u8*)ev, ev->size, 16);
                SysPacketEvent* packet = (SysPacketEvent*)ev;
                CL_ReadPacket(
                    packet,
                    &g_reliableStream,
                    &g_unreliableStream,
                    &g_sim.quantise,
                    g_elapsed);
            } break;

            case SYS_EVENT_INPUT:
            {
                SysInputEvent* inputEv = (SysInputEvent*)ev;
                Input_TestForAction(
					&g_inputActions,
					inputEv->value,
					inputEv->inputID,
					platformFrame);
				
				if (Input_CheckActionToggledOn(
					&g_inputActions,
					"Menu",
					platformFrame))
				{
					g_mainMenuOn = !g_mainMenuOn;
				}

            } break;
            case SYS_EVENT_SKIP: break;
        }
    }
}

internal i32 CL_IsCommandTickSensitive(i32 cmdType)
{
	switch (cmdType)
	{
		case CMD_TYPE_S2C_SESSION_SYNC: { return false; }
	}
	return true;
}

/////////////////////////////////////////////////////////////
// Configures this client's time-base relative
// to the server.
// Client will be ((ping / 2) + jitter delay) behind
// the server
/////////////////////////////////////////////////////////////
internal void CL_SetServerTick(i32 value)
{
	g_serverTick = value - APP_DEFAULT_JITTER_TICKS;
}

// Can be changed during command execute so always retreive from here:
internal i32 CL_GetServerTick()
{
	return g_serverTick;
}

internal i32 CL_ExecReliableCommand(
    SimScene* sim, Command* h, timeFloat deltaTime, i32 tickDiff)
{
    //APP_LOG(64, "CL exec input seq %d\n", h->sequence);

	switch (h->type)
	{
        case CMD_TYPE_S2C_BULK_SPAWN:
        {
            S2C_BulkSpawn* prj = (S2C_BulkSpawn*)h;
            APP_LOG(256, "CL Spawn Prj %d on SV tick %d (local sv tick diff %d. Cmd tick %d)\n",
                prj->def.factoryType,
				prj->def.base.tick,
				prj->def.base.tick - CL_GetServerTick(),
				prj->header.tick
            );
            // flip diff to specify fast forwarding
            i32 flags;
            f32 priority;
            Sim_ExecuteBulkSpawn(sim, &prj->def, -tickDiff, &flags, &priority);
        } break;
		case CMD_TYPE_S2C_RESTORE_ENTITY:
		{
			S2C_RestoreEntity* spawn = (S2C_RestoreEntity*)h;
			APP_LOG(64, "CL Spawn %d at %.3f, %.3f, %.3f\n",
				spawn->networkId, spawn->pos.x, spawn->pos.y, spawn->pos.z
			);
			
			SimEntSpawnData def = {};
			def.serial = spawn->networkId;
            def.birthTick = h->tick;
            def.factoryType = spawn->factoryType;
			def.pos = spawn->pos;
			def.velocity = spawn->vel;
            def.fastForwardTicks = -tickDiff;
			Sim_RestoreEntity(sim, &def);
		} break;
        case CMD_TYPE_S2C_REMOVE_ENTITY:
        {
            S2C_RemoveEntity* cmd = (S2C_RemoveEntity*)h;
            CLG_HandleEntityDeath(&g_sim, cmd->entityId);
            Sim_RemoveEntity(sim, cmd->entityId);
            //APP_PRINT(64, "CL Remove Ent %d\n", cmd->entityId);
        } break;
        case CMD_TYPE_S2C_REMOVE_ENTITY_GROUP:
        {
            S2C_RemoveEntityGroup* cmd = (S2C_RemoveEntityGroup*)h;
            for (i32 i = 0; i < cmd->numIds; ++i)
            {
                i32 serial = cmd->firstId + i;
                CLG_HandleEntityDeath(sim, serial);
                Sim_RemoveEntity(sim, serial);
            }
        } break;
        case CMD_TYPE_S2C_SESSION_SYNC:
        {
            S2C_Sync* sync = (S2C_Sync*)h;
			CL_SetServerTick(sync->header.tick);
			g_avatarSerial = sync->avatarEntityId;
            APP_PRINT(64, "CL Set avatar %d\n", g_avatarSerial);
            // Lets not do what the server tells us!
			//g_serverTick = sync->simTick - sync->jitterTickCount;
			APP_LOG(64, "/////////////////////////////////////////\n");
            APP_LOG(64, "CL Sync server sim tick %d\n", CL_GetServerTick());
			APP_LOG(64, "/////////////////////////////////////////\n");
        } break;
		
		
		default:
		{
			APP_PRINT(64, "CL Unknown command type %d\n", h->type);
		} break;
	}
    return 1;
}

internal void CL_RunReliableCommands(
    SimScene* sim, NetStream* stream, timeFloat deltaTime)
{
	ZEByteBuffer* b = &stream->inputBuffer;
	
	for (;;)
	{
		i32 sequence = stream->inputSequence;
		Command* h = Stream_FindMessageBySequence(
			b->start, b->Written(), sequence);
        // No commands to run
		if (!h) { break; }

		i32 diff = h->tick - CL_GetServerTick();
		//APP_LOG(128, "CL Exec Cmd %d: Cmd Tick %d, Sync Tick %d (diff %d), CL Tick %d\n",
		//	h->sequence, h->tick, CL_GetServerTick(), diff, g_ticks
		//);
		
		if (CL_IsCommandTickSensitive(h->type))
		{
			if (diff > 0)
			{
				APP_LOG(128, "\tCL Delaying execution of cmd %d until tick %d (diff %d)\n",
					h->sequence, h->tick, diff);
				// Drop out - next reliable command cannot be executed until we reach this frame
				break;
			}
			
			if (diff < 0)
			{
				APP_LOG(128,  "\tCL Fast forward cmd %d by %d frames\n",
					h->sequence, -(diff)
				);
			}
		}
		
		// Step queue counter forward as we are now executing
		stream->inputSequence++;
		
		i32 err = Cmd_Validate(h);
		ZE_ASSERT(err == ZE_ERROR_NONE, "Invalid command")
		if (CL_ExecReliableCommand(sim, h, deltaTime, diff))
        {
            Stream_DeleteCommand(b, h, 0);
        }
	}
}

internal void CL_RunUnreliableCommands(
    SimScene* sim, NetStream* stream, timeFloat deltaTime)
{
	ZEByteBuffer* b = &stream->inputBuffer;
    u8* read = b->start;
	//APP_LOG(128, "CL Run %d bytes of unreliable msgs\n",
    //    b->Written());
    //CL_LogCommandBuffer(b, "unreliable");

	while (read < b->cursor)
	{
		i32 sequence = stream->inputSequence;
		Command* h = (Command*)read;
        i32 err = Cmd_Validate(h);
		if (err != ZE_ERROR_NONE)
		{
			APP_PRINT(128, "CL Run unreliable - invalid cmd code %d\n", err);
            b->Clear(NO);
			return;
		}
        
		i32 diff = h->tick - CL_GetServerTick();
		//APP_LOG(128, "CL Exec Cmd %d: Cmd Tick %d, Sync Tick %d (diff %d), CL Tick %d\n",
		//	h->sequence, h->tick, CL_GetServerTick(), diff, g_ticks
		//)
		
		// If executed,   delete command from buffer
        i32 executed = 0;
        if (diff <= 0)
        {
            switch (h->type)
            {
                case CMD_TYPE_S2C_SYNC_ENTITY:
                {
                    S2C_EntitySync* cmd = (S2C_EntitySync*)h;
                    executed = CLG_SyncEntity(sim, cmd);
                } break;

                case CMD_TYPE_S2C_INPUT_RESPONSE:
                {
                    S2C_InputResponse* cmd = (S2C_InputResponse*)h;
                    CLG_SyncAvatar(sim, cmd);
                    executed = 1;
                } break;

                case CMD_TYPE_PING:
                {
                    CmdPing* cmd = (CmdPing*)h;
                    executed = 1;
                } break;

                default:
                {
                    APP_PRINT(64, "CL Unknown unreliable type %d\n", h->type);
                } break;
            }
        }
        
        if (executed)
        {
            Stream_DeleteCommand(b, h, 0);
        }
        else
        {
            read += h->size;
        }
    }
}

internal void CL_CalcPings(timeFloat deltaTime)
{
	g_ping = Ack_CalculateAverageDelay(&g_acks);
	g_jitter = (g_acks.delayMax - g_acks.delayMin);
}

void CL_Tick(ZEByteBuffer* sysEvents, timeFloat deltaTime, i64 platformFrame)
{
    APP_LOG(64, "*** CL TICK %d (Server Sync Tick %d. T %.3f) ***\n",
        g_ticks, g_serverTick, g_elapsed);
    CL_ReadSystemEvents(sysEvents, deltaTime, platformFrame);

    CL_CalcPings(deltaTime);
    
	CL_RunReliableCommands(&g_sim, &g_reliableStream, deltaTime);
    //CL_LogCommandBuffer(&g_unreliableStream.inputBuffer, "Unreliable input");
    CL_RunUnreliableCommands(&g_sim, &g_unreliableStream, deltaTime);

    // Update input
    CL_UpdateActorInput(&g_inputActions, &g_actorInput);
	// Create and store input to server
	C2S_Input cmd;
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
        g_serverTick,
        deltaTime);
	CL_StoreSentInputCommand(g_sentCommands, &cmd);
	// Run
    CLG_TickGame(&g_sim, deltaTime);
	g_ticks++;
	g_serverTick++;
    g_elapsed += deltaTime;
    CL_WritePacket(&g_sim.quantise, g_elapsed, &cmd);
}

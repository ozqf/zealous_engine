#ifndef CLIENT_INTERNAL_H
#define CLIENT_INTERNAL_H

/**
 * client internal shared head
 */
#include "client.h"
#include "client_render.h"
#include "../../sys_events.h"
#include "../shared/user.h"
#include "../../ze_common/ze_common.h"
#include "../../ze_common/ze_char_buffer.h"
#include "../app.h"
#include "../shared/commands.h"
#include "../shared/stream.h"

struct ClientHudState
{
	i32 hp;
	i32 ammo;
};

internal u32 g_clDebugFlags = 0
    //| CL_DEBUG_FLAG_DRAW_LOCAL_SERVER
    //| CL_DEBUG_FLAG_DRAW_REAL_LOCAL_POSITION
    //| CL_DEBUG_FLAG_NO_ENEMY_TICK
    //| CL_DEBUG_FLAG_NO_PLAYER_SMOOTHING
	//| CL_DEBUG_FLAG_DEBUG_CAMERA
;

internal ClientRenderSettings g_rendCfg;

internal i32 g_clientState = CLIENT_STATE_NONE;

internal i32 g_isRunning = 0;
internal SimScene g_sim;
internal i32 g_bHasSimSynced = NO;
internal Transform g_camera;
internal Vec3 g_testCameraDegrees = {};
internal timeFloat g_elapsed = 0;
internal timeFloat g_ping;
internal timeFloat g_jitter;
internal i32 g_bClientAlwaysRepredict = YES;

internal i32 g_avatarSerial = 0;
internal i32 g_userTargetSerial = 0;

#define CL_MAX_ALLOCATIONS 256
internal void* g_allocations[CL_MAX_ALLOCATIONS];
internal i32 g_bytesAllocated = 0;
internal i32 g_numAllocations = 0;
internal f32 g_avatarSmoothingRate = 0.8f;

internal Vec3 g_testHitPos = { 0, 2, 0 };
internal M4x4 g_matrix;

internal i32 g_interpolateRenderScene = 0;
//internal i32 g_tickEnemies = 1;

internal i32 g_bVerboseFrame = YES;

internal f32 g_debugSkipReportDistance;

internal f32 g_requestTick = 0;

// Menus
internal i32 g_mainMenuOn;

#define CL_MAX_INPUT_ACTIONS 256
internal InputAction g_inputActionItems[CL_MAX_INPUT_ACTIONS];
internal InputActionSet g_inputActions = {
    g_inputActionItems,
    0
};

//////////////////////////////////////////////
// Connection stuff
//////////////////////////////////////////////
internal NetStream g_reliableStream;
internal NetStream g_unreliableStream;
internal UserIds g_ids;
internal AckStream g_acks;
internal ZNetAddress g_serverAddress;
internal i32 g_udpSocketId;

//////////////////////////////////////////////
// Game sync/prediction
//////////////////////////////////////////////
internal i32 g_userInputSequence = 0;
internal i32 g_latestUserInputAck = 0;
internal Vec3 g_latestAvatarPos = {};
internal i32 g_bHasNewResponse = NO;
internal S2C_InputResponse g_lastInputResponse = {};

// Buffer transmitted inputs
internal C2S_Input g_sentCommands[CL_MAX_SENT_INPUT_COMMANDS];

//#define CL_MAX_RENDER_COMMANDS 1024
//internal RenderCommand* g_renderCommands;

internal SimActorInput g_actorInput = {};

#define CLI_MAX_RESPONSE_RECORDS 60

internal S2C_InputResponse g_serverResponses[CLI_MAX_RESPONSE_RECORDS];


internal i32 CL_ReadPacket(
    SysPacketEvent* ev,
    NetStream* reliableStream,
    NetStream* unreliableStream,
    QuantiseSet* quantise,
    timeFloat time);
internal void CLG_HandleEntityDeath(
	SimScene* sim, i32 serial);
internal void CL_SetServerTick(i32 value);
internal i32 CL_GetServerTick();
internal i32 CLG_SyncEntity(SimScene* sim, S2C_EntitySync* cmd);
internal void CLI_RecordServerResponse(
    S2C_InputResponse* list, S2C_InputResponse* item);

#endif // CLIENT_INTERNAL_H
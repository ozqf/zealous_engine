#pragma once


#include "../interface/app_interface.h"
#include "../interface/platform_interface.h"

#include "app.h"
#include "server/server.h"
#include "client/client.h"
#include "app_textures.h"
#include "../network/znet_interface.h"
#include "app_fake_socket.h"

#define APP_SESSION_TYPE_NONE 0
#define APP_SESSION_TYPE_SINGLE_PLAYER 1

// Access to platform info
internal AppPlatform g_platform = {};
internal ScreenInfo g_screenInfo;

/////////////////////////////////////////////////////////////////
// TIMING
internal i32 g_simFrameRate = 60;
internal f32 g_simFrameAcculator = 0;
internal u32 g_lastPlatformFrame = 0;

internal u32 g_renderCalls = 0;

/////////////////////////////////////////////////////////////////
// MEMORY

// TODO: Remove this
internal Heap g_heap;

internal i32 g_isRunningClient = 0;
internal i32 g_isRunningServer = 0;

// Client/Server input buffers
internal DoubleByteBuffer g_serverLoopback;
internal DoubleByteBuffer g_clientLoopback;

#define APP_MAX_MALLOCS 1024
internal MallocItem g_mallocItems[APP_MAX_MALLOCS];
internal MallocList g_mallocs;


/////////////////////////////////////////////////////////////////
// FAKE NETWORK QUALITY
// Should be 0 0 0 when not debugging (obviously)

internal i32 g_fakeLagMinMS = 150;
internal i32 g_fakeLagMaxMS = 200;
// 0 to 1 values.
internal f32 g_fakeLoss = 0.1f;

internal FakeSocket g_loopbackSocket;

#define MAX_WORLD_SCENE_ITEMS 2048
internal RenderScene g_worldScene;
internal RenderListItem g_worldSceneItems[MAX_WORLD_SCENE_ITEMS];

internal ZNetAddress g_localServerAddress;

/////////////////////////////////////////////////////////////////
// DEBUGGING

// Buffers for storing frame timing info
internal AppPerformanceStat g_performanceStats[APP_STAT_COUNT];

// Big string to write debug crap into
#define DEBUG_STRING_LENGTH 8192
#define DEBUG_NUM_STRINGS 8
internal char g_debugStrBuffer[DEBUG_STRING_LENGTH];
internal CharBuffer g_debugStr;
internal RendObj g_debugRendObjs[DEBUG_NUM_STRINGS];

#define MAX_DEBUG_SCENE_ITEMS 64
internal RenderScene g_debugScene;
internal RenderListItem g_debugSceneItems[MAX_DEBUG_SCENE_ITEMS];

#define APP_REND_FLAG_SERVER_SCENE (1 << 0)
#define APP_REND_FLAG_SERVER_TESTS (1 << 1)
#define APP_REND_FLAG_CLIENT_PREDICTIONS (1 << 2)

internal i32 g_debugRenderFlags = 0
    | APP_REND_FLAG_CLIENT_PREDICTIONS
;

#define APP_PRINT_FLAG_SPEEDS (1 << 0)
#define APP_PRINT_FLAG_SERVER (1 << 1)
#define APP_PRINT_FLAG_CLIENT (1 << 2)

internal i32 g_debugPrintFlags = 0
    | APP_PRINT_FLAG_SPEEDS
;

#define APP_MAX_DEBUG_STRING_POSITIONS 6
internal Vec3 g_debugStrPositions[APP_MAX_DEBUG_STRING_POSITIONS] =
{
    // Viewport Screen thirds across X
    { -1, 1 }, { -0.334f, 1 }, { 0.334f, 1 },
    { -1, 0 }, { -0.334f, 0 }, { 0.334f, 0 },
};

/////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
/////////////////////////////////////////////////////////////////
internal i32 App_StartSession(i32 sessionType);

void App_FatalError(char* msg, char* heading)
{
    printf("FATAL %s: %s\n", heading, msg);
    ILLEGAL_CODE_PATH
}

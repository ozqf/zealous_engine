#pragma once

#include "../../headers/sys_events.h"
#include "../../headers/ze_module_interfaces.h"

#include "app.h"
// #include "server/server.h"
// #include "client/client.h"
#include "game/game.h"
#include "app_fake_socket.h"

/////////////////////////////////////////////////////////////////
// FAKE NETWORK QUALITY
// Should be 0 0 0 when not debugging (obviously)

// 100 ms == approx 6 frames at 60fps
// 200 ms == approx 12 frames at 60fps
internal i32 g_fakeLagMinMS = 0;//50;//100;
internal i32 g_fakeLagMaxMS = 0;//100;//350;
// 0 to 1 values.
internal f32 g_fakeLoss = 0;//0.1f;

internal FakeSocket g_loopbackSocket;
internal i32 g_debugSocket;
internal u16 g_debugPort;

// Access to platform info
internal ze_platform_export g_platform = {};
internal f64 g_lastTimeSample;
internal ScreenInfo g_screenInfo;

/////////////////////////////////////////////////////////////////
// SIMULATION TIMING
internal i32 g_simFrameRate = 60;
internal f32 g_simFrameAcculator = 0;
internal i64 g_lastPlatformFrame = 0;

internal u32 g_renderCalls = 0;
internal u32 g_framesWritten = 0;
internal i32 g_apptick = 0;

/////////////////////////////////////////////////////////////////
// MEMORY

internal ZEDoubleBuffer g_gameBuffers;
internal ZEBuffer g_soundBuffer;

#define APP_MAX_MALLOCS 1024
internal MallocItem g_mallocItems[APP_MAX_MALLOCS];
internal MallocList g_mallocs;

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

#define APP_PRINT_FLAG_SPEEDS (1 << 0)
#define APP_PRINT_FLAG_SERVER (1 << 1)
#define APP_PRINT_FLAG_CLIENT (1 << 2)

internal i32 g_debugPrintFlags = 0
    | APP_PRINT_FLAG_SPEEDS
;
internal i32 g_debugLogFlags = 0
    | APP_LOG_CATEGORY_GAME
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
internal i32 App_StartSession(i32 sessionType, const char* mapName);
internal void App_StartTitle();

void App_FatalError(char* msg, char* heading)
{
    printf("FATAL %s: %s\n", heading, msg);
    ILLEGAL_CODE_PATH
}

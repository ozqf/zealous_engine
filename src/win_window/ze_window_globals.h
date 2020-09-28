#ifndef ZE_WINDOW_GLOBALS_H
#define ZE_WINDOW_GLOBALS_H

#include "../../lib/glfw3_vc2015/glfw3.h"
//#include "win32_header.h"
//#include "app/app.h"

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_byte_buffer.h"

#include "../ui/zui.h"

// Version of Opengl that will be requested
const i32 MAJOR_VERSION_REQ = 3;
const i32 MINOR_VERSION_REQ = 3;

#define SENTINEL 0xDEADBEEF
#if 0
struct ZqfThread
{
    i32 sentinel;
    char* label;
};

struct Win32_Thread
{
    ZqfThread header;
    // Windows specific
    DWORD threadId;
    HANDLE handle;
};
#endif

// Buffers
internal ZEBuffer g_drawListBuffer;
internal ZEBuffer g_drawDataBuffer;
internal ZEBuffer g_eventBuffer;

internal i32 g_bMouseCaptured = YES;
internal i32 g_bAppWantsMouseCaptured = YES;
internal i32 g_consoleActive = NO;

// recording mouse position
internal f64 g_mousePosX = 0;
internal f64 g_mousePosY = 0;
internal f64 g_mousePosNormalisedX = 0;
internal f64 g_mousePosNormalisedY = 0;
// recording mouse movement
internal f64 g_lastMouseSampleX = 0;
internal f64 g_lastMouseSampleY = 0;
internal f64 g_mouseAccumulatorSampleX = 0;
internal f64 g_mouseAccumulatorSampleY = 0;

internal ze_platform_export g_platform = {};
//internal ZRRenderer g_renderer = {};
internal volatile i32 g_bExit = NO;
// dimensions of the current window
internal ScreenInfo g_windowSize;
// monitor dimensions - read at load time so won't update
// if screen moves around
internal ScreenInfo g_monitorSize;

// cap fps
internal i32 g_maxFPS = 0;

internal i32 g_bWindowed = YES;
// Current res mode
internal i32 g_pendingScrMode = 0;
internal i32 g_resolutionsX[ZW_NUM_16X9_RESOLUTIONS] =
{ 1024, 1280, 1366, 1600, 1920 };
internal i32 g_resolutionsY[ZW_NUM_16X9_RESOLUTIONS] =
{ 576, 720, 768, 900, 1080 };
internal const i32 g_numModes = 5;

internal i32 g_bRestart = NO;
internal GLFWwindow* g_window;

#define ZE_WINDOW_NUM_CONSOLE_OBJECTS
internal ZUIObject g_consoleUIObjs[2];
internal ZUIScreen g_consoleScene;
internal i32 g_lastAppFrameNumber = 0;

#endif // ZE_WINDOW_GLOBALS_H
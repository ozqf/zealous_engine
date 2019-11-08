#ifndef ZE_WINDOW_GLOBALS_H
#define ZE_WINDOW_GLOBALS_H

#include "../../lib/glfw3_vc2015/glfw3.h"
//#include "win32_header.h"
//#include "app/app.h"

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_byte_buffer.h"

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
static ZEByteBuffer g_drawListBuffer;
static ZEByteBuffer g_drawDataBuffer;
static ZEByteBuffer g_eventBuffer;

static i32 g_bMouseCaptured = NO;
static i32 g_consoleActive = NO;

static f64 g_lastMouseSampleX = 0;
static f64 g_lastMouseSampleY = 0;
static f64 g_mouseAccumulatorSampleX = 0;
static f64 g_mouseAccumulatorSampleY = 0;

static ze_platform_export g_platform = {};
static ZRRenderer g_renderer = {};
static volatile i32 g_bExit = NO;
static ScreenInfo g_scrInfo;

internal GLFWwindow* g_window;

#endif // ZE_WINDOW_GLOBALS_H
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

static ze_kernel_export g_platform = {};
static ZRRenderer g_renderer = {};
static volatile i32 g_bExit = NO;

//internal Win32_Thread g_appThread;
internal GLFWwindow* g_window;
//internal App g_app;
//internal ZRRenderer g_renderer;
//internal ScreenInfo g_screenInfo;

//internal u8 g_platformEventsBuf

internal ZEByteBuffer g_events;

#endif // ZE_WINDOW_GLOBALS_H
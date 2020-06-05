#ifndef ZE_WIN64_WINDOW_H
#define ZE_WIN64_WINDOW_H
/*
Zealous Engine Windows renderer
*/
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

//#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

// glfw must come AFTER windows to avoid macro redefinition error
#include "../../lib/glad/glad.h"
#include "../../lib/glfw3_vc2015/glfw3.h"

#include "../ze_module_interfaces.h"
#include "../ze_common/ze_common.h"

#include "../zqf_renderer.h"

#define ZW_NUM_16X9_RESOLUTIONS 5

#include "ze_window_globals.h"
#include "ze_window_callbacks.h"

static void ZR_Error(const char* msg)
{
    g_platform.Error((char*)msg);
}

static i32 WindowImpl_Init()
{
    //g_platform.Warning("Hello from window DLL", "666");
    ZE_SetFatalError(ZR_Error);

    i32 bytes = MegaBytes(1);
    g_drawListBuffer = Buf_FromMalloc(malloc(bytes), bytes);
    g_drawDataBuffer = Buf_FromMalloc(malloc(bytes), bytes);
    g_eventBuffer = Buf_FromMalloc(malloc(bytes), bytes);

    //////////////////////////////////////////////////////////////////
    // GLFW - Build opengl context, window and callbacks
    //////////////////////////////////////////////////////////////////

    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }
    
    #if 1 // normal, resolution locked window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION_REQ);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION_REQ);
    
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Setup window resolution
    // tiny window
    const i32 scrMode = 0;
    // bigger window
    //const i32 scrMode = 3;
    
    const i32 scrWidth = g_resolutionsX[scrMode];
    const i32 scrHeight = g_resolutionsY[scrMode];
    g_scrInfo.width = scrWidth;
    g_scrInfo.height = scrHeight;
    g_scrInfo.aspectRatio = (f32)scrWidth / (f32)scrHeight;
    printf("Aspect ratio %.3f\n", g_scrInfo.aspectRatio);

    // Create
    g_window = glfwCreateWindow(scrWidth, scrHeight, "Zealous Engine", NULL, NULL);
    #endif

    if (!g_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
	
    // Init callbacks and events buffer
    ZR_InitCallbacks(g_window);

    glfwMakeContextCurrent(g_window);

    //gladLoadGL(glfwGetProcAddress);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//////////////////////////////////////////////////////////////////
    // Opengl enabled from here on
    //////////////////////////////////////////////////////////////////
    
    i32 majorVer;
    i32 minorVer;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVer);
    glGetIntegerv(GL_MINOR_VERSION, &minorVer);
    printf("Requested Opengl ver %d.%d\n", MAJOR_VERSION_REQ, MINOR_VERSION_REQ);
    printf("Opengl ver read as %d.%d\n", majorVer, minorVer);
    
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, scrWidth, scrHeight);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwSwapBuffers(g_window);

    ZRPlatform platform = {};
    platform.Allocate = g_platform.Allocate;
    platform.Free = g_platform.Free;
    platform.QueryClock = g_platform.QueryClock;
    g_renderer = ZR_Link(platform);

    ErrorCode err = g_renderer.Init(g_scrInfo.width, g_scrInfo.height);
    if (err != ZE_ERROR_NONE)
    {
        g_platform.Error("Error initialising renderer");
        return ZE_ERROR_UNKNOWN;
    }

	return ZE_ERROR_NONE;
}

static void ZR_PollMouseForApp()
{
    f64 posX, posY;
    glfwGetCursorPos(g_window, &posX, &posY);
    f64 diffX = posX - g_lastMouseSampleX;
    f64 diffY = posY - g_lastMouseSampleY;
    i32 bMouseMoved = NO;
    if (diffX != 0)
    {
        bMouseMoved = YES;
        g_lastMouseSampleX = posX;
        if (Win_IsCursorDisabled())
        {
            g_mouseAccumulatorSampleX += (diffX / g_scrInfo.width);
        }
    }
    if (diffY != 0)
    {
        bMouseMoved = YES;
        g_lastMouseSampleY = posY;
        if (Win_IsCursorDisabled())
        {
            g_mouseAccumulatorSampleY += (diffY / g_scrInfo.height);
        }
    }
    #if 0 // dump mouse input samples
    if (bMouseMoved == YES)
    {
        printf("Mouse Pos: %.3f, %.3f\n", posX, posY);
        printf("\tNormalised: %.3f, %.3f\n",
            posX / g_scrInfo.width, posY / g_scrInfo.height);
        printf("\tMouse accumulator: %.3f, %.3f\n",
            g_mouseAccumulatorSampleX, g_mouseAccumulatorSampleY);
    }
    #endif
}

static void ZR_PollInput()
{
    g_platform.LockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
	glfwPollEvents();
    g_platform.UnlockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
}

static void WindowImpl_Acquire_AppDrawBuffers(ZEByteBuffer** listBuf, ZEByteBuffer** dataBuf)
{
    g_platform.LockMutex(ZE_MUTEX_DRAW_QUEUE, 0);
    *listBuf = &g_drawListBuffer;
    *dataBuf = &g_drawDataBuffer;
}

static void WindowImpl_Release_AppDrawBuffers()
{
    g_platform.UnlockMutex(ZE_MUTEX_DRAW_QUEUE, 0);
}

static void WindowImpl_Acquire_EventBuffer(ZEByteBuffer** buf)
{
    g_platform.LockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
    *buf = &g_eventBuffer;
    // Write current mouse state for app to read
    ZR_PollMouseForApp();
    //if (g_mouseAccumulatorSampleX != 0 || g_mouseAccumulatorSampleY != 0)
    //{
        // inputs take integers...
        i32 mouseMoveIntX = (i32)(g_mouseAccumulatorSampleX * Z_INPUT_MOUSE_SCALAR);
        i32 mouseMoveIntY = (i32)(g_mouseAccumulatorSampleY * Z_INPUT_MOUSE_SCALAR);
        //printf("WIN mouse moved %d / %d\n", mouseMoveIntX, mouseMoveIntY);
        Sys_WriteInputEvent(&g_eventBuffer, Z_INPUT_CODE_MOUSE_MOVE_X, mouseMoveIntX);
        Sys_WriteInputEvent(&g_eventBuffer, Z_INPUT_CODE_MOUSE_MOVE_Y, mouseMoveIntY);
        // Clear for next sample
        g_mouseAccumulatorSampleX = 0;
        g_mouseAccumulatorSampleY = 0;
    //}
}

static void WindowImpl_Release_EventBuffer()
{
    g_platform.UnlockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
}

static void* WindowImpl_GetAssetDB()
{
    return g_renderer.GetAssetDB();
}

static i32 WindowImpl_MainLoop()
{
    f64 startFrameMS = 0, endFrameMS = 0, totalMS = 0;
    while(!glfwWindowShouldClose(g_window))
    {
        startFrameMS = g_platform.QueryClock();
        ZEByteBuffer* list;
        ZEByteBuffer* data;
        g_platform.Acquire_AppDrawBuffers(&list, &data);
        // Draw
        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // Forward renderer currently bricked!
        //g_renderer.DrawFrameForward(list, data, g_scrInfo);
        g_renderer.DrawFrameDeferred(list, data, g_scrInfo);
        // Finish Frame
        g_platform.Release_AppDrawBuffers();
        f64 swapStart = g_platform.QueryClock();
        glfwSwapBuffers(g_window);
        f64 swapEnd = g_platform.QueryClock();
        g_renderer.UpdateStats(
            swapEnd - swapStart,
            totalMS
            );
        ZR_PollInput();
        endFrameMS = g_platform.QueryClock();
        totalMS = endFrameMS - startFrameMS;
    }
    return ZE_ERROR_NONE;
}

extern "C"
ze_window_export __declspec(dllexport) ZE_LinkToWindowModule(ze_platform_export platform)
{
	g_platform = platform;
	ze_window_export result = {};
	result.Init = WindowImpl_Init;
    result.MainLoop = WindowImpl_MainLoop;
    result.Acquire_EventBuffer = WindowImpl_Acquire_EventBuffer;
    result.Release_EventBuffer = WindowImpl_Release_EventBuffer;
    result.Acquire_AppDrawBuffers = WindowImpl_Acquire_AppDrawBuffers;
    result.Release_AppDrawBuffers = WindowImpl_Release_AppDrawBuffers;
    result.GetAssetDB = WindowImpl_GetAssetDB;
    result.sentinel = ZE_SENTINEL;
	return result;
}

#endif // ZE_WIN64_WINDOW_H
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

#include "ze_window_globals.h"
#include "ze_window_callbacks.h"

static void ZR_Error(const char* msg)
{
    g_platform.Error((char*)msg);
}

static i32 ZR_Init()
{
    //g_platform.Warning("Hello from window DLL", "666");
    ZE_SetFatalError(ZR_Error);

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

    const i32 scrWidth = 1280;
    const i32 scrHeight = 768;
    g_scrInfo.width = scrWidth;
    g_scrInfo.height = scrHeight;
    g_scrInfo.aspectRatio = scrWidth / scrHeight;

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
    glClearColor(1, 0, 1, 1);
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

static i32 ZR_MainLoop()
{
    while(!glfwWindowShouldClose(g_window))
    {
        ZEByteBuffer* list;
        ZEByteBuffer* data;
        g_platform.Acquire_AppDrawBuffers(&list, &data);
        // Draw
        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        g_renderer.DrawFrame(list, data, g_scrInfo);
        // Finish Frame
        g_platform.Release_AppDrawBuffers();
        glfwSwapBuffers(g_window);

        g_platform.LockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
		glfwPollEvents();
        g_platform.UnlockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
    }
    return ZE_ERROR_NONE;
}

extern "C"
ze_window_export __declspec(dllexport) ZE_LinkToWindowModule(ze_platform_export platform)
{
	g_platform = platform;
	ze_window_export result = {};
	result.Init = ZR_Init;
    result.MainLoop = ZR_MainLoop;
	return result;
}

#endif // ZE_WIN64_WINDOW_H
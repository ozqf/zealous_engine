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
#include "../ze_common/ze_string_parse.h"

#include "../zqf_renderer/zrgl.h"

#define ZW_NUM_16X9_RESOLUTIONS 5

static void Window_EnqueueTextCommand(char* str);

#include "ze_window_globals.h"
#include "ze_window_callbacks.h"

i32 g_framesWritten = 0;

static void ZR_Error(const char* msg)
{
    g_platform.Error((char*)msg);
}

static void Window_EnqueueTextCommand(char* str) 
{
	printf(">> Window read cmd \"%s\"\n", str);
	const i32 maxTokens = 64;
	char* tokens[maxTokens];
	i32 len = ZE_StrLen(str);
	const i32 bufSize = 512;
	char buf[bufSize];
	if (len > bufSize)
	{
		printf("WIN - cmd string too long: %d max %d\n", len, bufSize);
		return;
	}
	i32 numTokens = ZE_ReadTokens(str, buf, tokens, maxTokens);

    // try and execute ourselves. If not pass off to platform
    if (numTokens == 2 && ZE_CompareStrings(tokens[0], "VID") == 0)
    {
        i32 mode = ZE_AsciToInt32(tokens[1]);
        if (mode < 0) { mode = 0; }
        if (mode >= g_numModes) { mode = g_numModes - 1; }
        g_pendingScrMode = mode;
        g_bRestart = YES;
        return;
    }
    if (numTokens == 2 && ZE_CompareStrings(tokens[0], "WINDOWED") == 0)
    {
        i32 value = ZE_AsciToInt32(tokens[1]);
        if (value != 0 && value != 1) { return; }
        if (g_bWindowed == value) { return; }
        g_bWindowed = value;
        g_bRestart = YES;
        return;
    }
    if (ZE_CompareStrings(str, "EXIT") == 0
        || ZE_CompareStrings(str, "QUIT") == 0)
	{
        printf("WIN - exiting\n");
        glfwSetWindowShouldClose(g_window, YES);
		return;
	}
    // Help and version commands should fall through
    // let other modules list their commands too!
    if (ZE_CompareStrings(str, "HELP") == 0)
	{
        printf("=== command list ===\n");
        printf("EXIT / QUIT - shutdown game, surprisingly enough\n");
        printf("VID <integer> - set windowed resolution mode\n");
        printf("WINDOWED <0 or 1> - if 0, activate borderless fullscreen\n");
        printf("VERSION - list module build dates\n");
	}
    if (ZE_CompareStrings(tokens[0], "VERSION") == 0)
	{
		printf("WINDOW Built %s: %s\n", __DATE__, __TIME__);
	}
    
    i32 rendererResponse = ZRGL_ExecTextCommand(str, len, tokens, numTokens);
    if (rendererResponse == YES) { return; }

    g_platform.ExecTextCommand(str, len, (const char**)tokens, numTokens);
}

static i32 WindowImpl_IsMouseCaptured()
{
    return Win_IsCursorDisabled();
}

static void WindowImpl_SetMouseCaptured(bool flag)
{
    // Mouse state change must be done on the window thread!
    // record and check when polling input
    g_bAppWantsMouseCaptured = flag;
}

static void Window_Error(const char* msg)
{
    // casting down from const...
    g_platform.Error((char*)msg);
}

// Create debug console display screen
static void Window_InitConsoleScreen()
{
    ZUI_Init(Window_Error, (ZRAssetDB*)g_platform.GetAssetDB());
    g_consoleScene = {};
	g_consoleScene.objects = g_consoleUIObjs;
	g_consoleScene.maxObjects = 2;
    g_consoleScene.state = ZUI_SCREEN_STATE_ON;

    ZUIObject* obj = NULL;
    obj = &g_consoleUIObjs[g_consoleScene.numObjects++];
    *obj = {};
    obj->radiusInChars = { 32, 2 };
    obj->pos.x = 0;
    obj->pos.y = 0.9f;
    // label will be updated per frame anyway
    obj->label = "Testing McTest Face.";
    obj->charSize = ZR_CharScreenSizeDefault();
    obj->offColour = COLOUR_WHITE;
    obj->onColour = COLOUR_WHITE;
    obj->bgColour = COLOUR_BLACK;
}

//////////////////////////////////////////////////////////////////
// Create Window, gl context and renderer
//////////////////////////////////////////////////////////////////
static ErrorCode Window_SpawnWindow()
{
    printf("WINDOW - spawn window\n");
    //////////////////////////////////////////////////////////////////
    // GLFW - Build opengl context, window and callbacks
    //////////////////////////////////////////////////////////////////
    glfwSetErrorCallback(error_callback);
    
    // Check glfw is okay
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }
    // global window settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MAJOR_VERSION_REQ);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MINOR_VERSION_REQ);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // grab current monitor res
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    i32 scrWidth = mode->width;
    i32 scrHeight = mode->height;
    i32 scrMode = g_pendingScrMode;

    // set monitor size - necessary to get the ratio between
    // full screen size and window size for mouse input
    g_monitorSize.width = scrWidth;
    g_monitorSize.height = scrHeight;
    g_monitorSize.aspectRatio = (f32)scrWidth / (f32)scrHeight;

    if (g_bWindowed == YES)
    {
        // Resolution locked window
        // Disable decoration and set window size to desktop size
        // to have borderless fullscreen
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        scrWidth = g_resolutionsX[scrMode];
        scrHeight = g_resolutionsY[scrMode];
    }
    else
    {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }
    // record screen size
    g_windowSize.width = scrWidth;
    g_windowSize.height = scrHeight;
    g_windowSize.aspectRatio = (f32)scrWidth / (f32)scrHeight;
    printf("Aspect ratio %.3f\n", g_windowSize.aspectRatio);
    // Create!
    g_window = glfwCreateWindow(scrWidth, scrHeight, "Zealous Engine", NULL, NULL);
    

    if (!g_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // Init callbacks and events buffer
    ZR_InitCallbacks(g_window);
    glfwMakeContextCurrent(g_window);
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

    // Initialise the renderer itself
    ErrorCode err = ZRGL_Init(g_windowSize.width, g_windowSize.height);
    if (err != ZE_ERROR_NONE)
    {
        g_platform.Error("Error initialising renderer");
        return ZE_ERROR_UNKNOWN;
    }

    // do a scan for either default assets or stuff an app
    // has loaded.
    ZRGL_CheckForUploads(NO);
    return ZE_ERROR_NONE;
}

static void Window_Restart()
{
    printf("WINDOW - restart\n");
    // Kill window
    glfwDestroyWindow(g_window);
    g_window = NULL;
    // Inform the asset db to clear all handles
    ZRAssetDB* db = (ZRAssetDB*)g_platform.GetAssetDB();
    db->VidRestart(db);
    Window_SpawnWindow();
}

static i32 WindowImpl_Init()
{
    printf("WINDOW - init\n");
    //g_platform.Warning("Hello from window DLL", "666");
    // This NEVER happens honest gov.
    ZE_SetFatalError(ZR_Error);

    printf("WINDOW - allocate draw list + event buffers\n");
    i32 bytes = MegaBytes(1);
    g_drawListBuffer = Buf_FromMalloc(malloc(bytes), bytes);
    g_drawDataBuffer = Buf_FromMalloc(malloc(bytes), bytes);
    g_eventBuffer = Buf_FromMalloc(malloc(bytes), bytes);

    Window_InitConsoleScreen();

    // create export for renderer and link up
    // but DO NOT INIT - init when window is created
    ZRPlatform platform = {};
    // platform.Allocate = g_platform.Allocate;
    // platform.Free = g_platform.Free;
    platform.Error = g_platform.Error;
    platform.QueryClock = g_platform.QueryClock;
    platform.GetAssetDB = g_platform.GetAssetDB;
    platform.DebugBreak = g_platform.DebugBreak;
    ZRGL_Link(platform);

    return Window_SpawnWindow();
}

static void ZR_PollMouseForApp()
{
    f64 posX, posY;
    if (g_window == NULL)
    {
        return;
    }
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
            g_mouseAccumulatorSampleX += (diffX / g_windowSize.width);
        }
    }
    if (diffY != 0)
    {
        bMouseMoved = YES;
        g_lastMouseSampleY = posY;
        if (Win_IsCursorDisabled())
        {
            g_mouseAccumulatorSampleY += (diffY / g_windowSize.height);
        }
    }
    #if 0 // dump mouse input samples
    if (bMouseMoved == YES)
    {
        printf("Mouse Pos: %.3f, %.3f\n", posX, posY);
        printf("\tNormalised: %.3f, %.3f\n",
            posX / g_windowSize.width, posY / g_windowSize.height);
        printf("\tMouse accumulator: %.3f, %.3f\n",
            g_mouseAccumulatorSampleX, g_mouseAccumulatorSampleY);
    }
    #endif
}

static void ZR_PollInput()
{
    // grab input buffer and poll for events. handled by callbacks
    g_platform.LockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
	glfwPollEvents();
    if (g_bAppWantsMouseCaptured != g_bMouseCaptured)
    {
        g_bMouseCaptured = g_bAppWantsMouseCaptured;
        Window_ApplyMouseState(g_window);
    }
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
    if (g_window != NULL)
    {
        ZR_PollMouseForApp();
        // inputs take integers...
        Vec2 scrRatio;
        scrRatio.x = (f32)g_monitorSize.width / (f32)g_windowSize.width;
        scrRatio.y = (f32)g_monitorSize.height / (f32)g_windowSize.height;
        i32 mouseMoveIntX = (i32)(g_mouseAccumulatorSampleX * Z_INPUT_MOUSE_SCALAR);
        i32 mouseMoveIntY = (i32)(g_mouseAccumulatorSampleY * Z_INPUT_MOUSE_SCALAR);
        f32 normalisedMoveX = (f32)mouseMoveIntX / scrRatio.x;
        f32 normalisedMoveY = (f32)mouseMoveIntY / scrRatio.y;
        //printf("WIN normalised mouse: %f, %f\n", normalisedMoveX, normalisedMoveY);
        //printf("WIN mouse moved %d / %d\n", mouseMoveIntX, mouseMoveIntY);
        Sys_WriteInputEvent(
            &g_eventBuffer,
            Z_INPUT_CODE_MOUSE_MOVE_X,
            mouseMoveIntX,
            normalisedMoveX);
        Sys_WriteInputEvent(
            &g_eventBuffer,
            Z_INPUT_CODE_MOUSE_MOVE_Y,
            mouseMoveIntY,
            normalisedMoveY);
        // Clear for next sample
        g_mouseAccumulatorSampleX = 0;
        g_mouseAccumulatorSampleY = 0;
    }
}

static void WindowImpl_Release_EventBuffer()
{
    g_platform.UnlockMutex(ZE_MUTEX_WINDOW_EVENTS, 0);
}

static void Window_Tick()
{
    f64 startFrameMS = 0, endFrameMS = 0, totalMS = 0;
    startFrameMS = g_platform.QueryClock();
    ZEByteBuffer* list;
    ZEByteBuffer* data;
    g_platform.Acquire_AppDrawBuffers(&list, &data);
    list->Clear(NO);
    data->Clear(NO);

    // setup frame data header and pass to app so it can write its scenes
    ZRViewFrame* frame = (ZRViewFrame*)list->cursor;
    list->cursor += sizeof(ZRViewFrame);
    *frame = {};
    frame->sentinel = ZR_SENTINEL;
    frame->frameNumber = g_framesWritten++;
    frame->list = list;
    frame->data = data;

    g_platform.AppWriteDraw(frame);

#if 1
    if (g_consoleActive == YES)
    {
        //ZRViewFrame* frame = (ZRViewFrame*)list->start;
        // TODO: This render buffer write stuff sounds bad:
        // Check that we haven't written to this buffer before!
        // Otherwise we will repeatedly write duplicate draw data into the buffer
        // before the app resets it!
        if (g_lastAppFrameNumber != frame->frameNumber)
        {
            g_lastAppFrameNumber = frame->frameNumber;
            g_consoleUIObjs[0].label = g_consoleInputBuffer;
            ZUI_WriteScreenForRender(frame, &g_consoleScene, list, data);
        }
    }
#endif
    // Draw
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ZRGL_DrawFrame(list, data, g_windowSize);
    // Finish Frame
    g_platform.Release_AppDrawBuffers();
    f64 swapStart = g_platform.QueryClock();
    glfwSwapBuffers(g_window);
    f64 swapEnd = g_platform.QueryClock();
    ZRGL_UpdateStats(
        swapEnd - swapStart,
        totalMS
        );
    ZR_PollInput();
    endFrameMS = g_platform.QueryClock();
    totalMS = endFrameMS - startFrameMS;
    if (g_bRestart == YES)
    {
        g_bRestart =  NO;
        Window_Restart();
    }
}

static i32 WindowImpl_MainLoop()
{
    f64 lastTickTime = 0;
    while(!glfwWindowShouldClose(g_window))
    {
        f64 time = g_platform.QueryClock();
        i32 tick = YES;
        #if 0
        if (g_maxFPS > 0)
        {
            // TODO: janky?
            f64 frameInterval = 1.0f / (f32)g_maxFPS;
            f64 diff = time - lastTickTime;
            if (diff < frameInterval)
            {
                tick = NO;
            }
        }
        #endif
        if (tick == YES)
        {
            lastTickTime = time;
            Window_Tick();
        }
        //g_platform.DebugBreak();
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
    result.IsMouseCaptured = WindowImpl_IsMouseCaptured;
    result.SetMouseCaptured = WindowImpl_SetMouseCaptured;
    result.sentinel = ZE_SENTINEL;
	return result;
}

#endif // ZE_WIN64_WINDOW_H
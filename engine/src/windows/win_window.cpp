#include "ze_windows.h"

// glfw must come AFTER windows to avoid macro redefinition error
#include "../../../lib/glad/glad.h"
#include "../../../lib/glfw3_vc2015/glfw3.h"

// Version of Opengl that will be requested
const i32 MAJOR_VERSION_REQ = 3;//4;
const i32 MINOR_VERSION_REQ = 3;

internal GLFWwindow *g_window;
/*
// Current res mode
internal i32 g_pendingScrMode = 2;
// 16/9 res modes:
internal i32 g_resolutionsX[ZW_NUM_16X9_RESOLUTIONS] =
{ 1024, 1280, 1366, 1600, 1920 };
internal i32 g_resolutionsY[ZW_NUM_16X9_RESOLUTIONS] =
{ 576, 720, 768, 900, 1080 };
internal const i32 g_numModes = 5;
*/
internal i32 g_pendingWidth = 1366; //1024;
internal i32 g_pendingHeight = 768; //576;

internal i32 g_bWindowed = FALSE;

internal i32 g_monitorSize[2];
internal f32 g_monitorAspect;

internal i32 g_windowSize[2];
internal f32 g_windowAspect;

internal i32 g_requestedMonitorIndex = 0;

internal i32 g_bLeftShiftOn = NO;
internal i32 g_bRightShiftOn = NO;

ze_internal ZEBuffer g_events;

// TODO: App thread can cause a 'GLFW is not initialised' error here
// by call into main thread during shutdown. Make sure App has closed before
// closing GLFW
static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
	Platform_Fatal(description);
}

static zeInputCode Win_GlfwToZEKey(i32 glfwKeyCode)
{
    switch (glfwKeyCode)
    {
        // mouse events handled via separate callbacks
        //case GLFW_KEY_: return Z_INPUT_CODE_NULL;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_1;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_2;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_3;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_4;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_5;
        //case GLFW_KEY_: return Z_INPUT_CODE_MWHEELUP;
        //case GLFW_KEY_: return Z_INPUT_CODE_MWHEELDOWN;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_POS_X;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_POS_Y;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_MOVE_X;
        //case GLFW_KEY_: return Z_INPUT_CODE_MOUSE_MOVE_Y;
        case GLFW_KEY_A: return Z_INPUT_CODE_A;
        case GLFW_KEY_B: return Z_INPUT_CODE_B;
        case GLFW_KEY_C: return Z_INPUT_CODE_C;
        case GLFW_KEY_D: return Z_INPUT_CODE_D;
        case GLFW_KEY_E: return Z_INPUT_CODE_E;
        case GLFW_KEY_F: return Z_INPUT_CODE_F;
        case GLFW_KEY_G: return Z_INPUT_CODE_G;
        case GLFW_KEY_H: return Z_INPUT_CODE_H;
        case GLFW_KEY_I: return Z_INPUT_CODE_I;
        case GLFW_KEY_J: return Z_INPUT_CODE_J;
        case GLFW_KEY_K: return Z_INPUT_CODE_K;
        case GLFW_KEY_L: return Z_INPUT_CODE_L;
        case GLFW_KEY_M: return Z_INPUT_CODE_M;
        case GLFW_KEY_N: return Z_INPUT_CODE_N;
        case GLFW_KEY_O: return Z_INPUT_CODE_O;
        case GLFW_KEY_P: return Z_INPUT_CODE_P;
        case GLFW_KEY_Q: return Z_INPUT_CODE_Q;
        case GLFW_KEY_R: return Z_INPUT_CODE_R;
        case GLFW_KEY_S: return Z_INPUT_CODE_S;
        case GLFW_KEY_T: return Z_INPUT_CODE_T;
        case GLFW_KEY_U: return Z_INPUT_CODE_U;
        case GLFW_KEY_V: return Z_INPUT_CODE_V;
        case GLFW_KEY_W: return Z_INPUT_CODE_W;
        case GLFW_KEY_X: return Z_INPUT_CODE_X;
        case GLFW_KEY_Y: return Z_INPUT_CODE_Y;
        case GLFW_KEY_Z: return Z_INPUT_CODE_Z;
        case GLFW_KEY_SPACE: return Z_INPUT_CODE_SPACE;
        case GLFW_KEY_LEFT_SHIFT: return Z_INPUT_CODE_LEFT_SHIFT;
        case GLFW_KEY_RIGHT_SHIFT: return Z_INPUT_CODE_RIGHT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return Z_INPUT_CODE_LEFT_CONTROL;
        case GLFW_KEY_RIGHT_CONTROL: return Z_INPUT_CODE_RIGHT_CONTROL;
        case GLFW_KEY_ESCAPE: return Z_INPUT_CODE_ESCAPE;
        case GLFW_KEY_ENTER: return Z_INPUT_CODE_RETURN;
        case GLFW_KEY_KP_ENTER: return Z_INPUT_CODE_ENTER;
        case GLFW_KEY_0: return Z_INPUT_CODE_0;
        case GLFW_KEY_1: return Z_INPUT_CODE_1;
        case GLFW_KEY_2: return Z_INPUT_CODE_2;
        case GLFW_KEY_3: return Z_INPUT_CODE_3;
        case GLFW_KEY_4: return Z_INPUT_CODE_4;
        case GLFW_KEY_5: return Z_INPUT_CODE_5;
        case GLFW_KEY_6: return Z_INPUT_CODE_6;
        case GLFW_KEY_7: return Z_INPUT_CODE_7;
        case GLFW_KEY_8: return Z_INPUT_CODE_8;
        case GLFW_KEY_9: return Z_INPUT_CODE_9;
        case GLFW_KEY_UP: return Z_INPUT_CODE_UP;
        case GLFW_KEY_DOWN: return Z_INPUT_CODE_DOWN;
        case GLFW_KEY_LEFT: return Z_INPUT_CODE_LEFT;
        case GLFW_KEY_RIGHT: return Z_INPUT_CODE_RIGHT;
        case GLFW_KEY_F1: return Z_INPUT_CODE_F1;
        case GLFW_KEY_F2: return Z_INPUT_CODE_F2;
        case GLFW_KEY_F3: return Z_INPUT_CODE_F3;
        case GLFW_KEY_F4: return Z_INPUT_CODE_F4;
        case GLFW_KEY_F5: return Z_INPUT_CODE_F5;
        case GLFW_KEY_F6: return Z_INPUT_CODE_F6;
        case GLFW_KEY_F7: return Z_INPUT_CODE_F7;
        case GLFW_KEY_F8: return Z_INPUT_CODE_F8;
        case GLFW_KEY_F9: return Z_INPUT_CODE_F9;
        case GLFW_KEY_F10: return Z_INPUT_CODE_F10;
        case GLFW_KEY_F11: return Z_INPUT_CODE_F11;
        case GLFW_KEY_F12: return Z_INPUT_CODE_F12;
		case 162: return Z_INPUT_CODE_BACKSLASH;
		case GLFW_KEY_BACKSLASH: return Z_INPUT_CODE_BACKSLASH;
        default:
        printf("Found no match for GLFW Key %d\n", glfwKeyCode);
        return Z_INPUT_CODE_NULL;
    }
}

ze_external ZScreenInfo Window_GetInfo()
{
    ZScreenInfo info = {};
    info.width = g_windowSize[0];
    info.height = g_windowSize[1];
    info.aspect = g_windowAspect;
    return info;
}

ze_external void Window_Shutdown()
{
    glfwSetWindowShouldClose(g_window, GLFW_TRUE);
    ZEngine_BeginShutdown();
}

ZCMD_CALLBACK(Exec_Windowed)
{
    if (numTokens < 2)
    {
        return;
    }
    i32 flag = ZStr_AsciToInt32(tokens[1]);
    printf("Windowed: %d\n", flag);
}

//////////////////////////////////////////////////////////////////
// GLFW Callbacks
//////////////////////////////////////////////////////////////////

/**
 * Handle inputs within this module. If they are handled, event is not
 * passed to app module.
 */
static i32 handle_engine_key_event(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    i32 bConsoleIgnore = NO;
    if (key == GLFW_KEY_LEFT_SHIFT)
    {
        g_bLeftShiftOn = (action == GLFW_PRESS);
        bConsoleIgnore = YES;
    }
    if (key == GLFW_KEY_RIGHT_SHIFT)
    {
        g_bRightShiftOn = (action == GLFW_PRESS);
        bConsoleIgnore = YES;
    }
    
    // (not) tilde key console toggle:
    // escape belongs to the App
    i32 bConsoleOn = ZCmdConsole_GetInputEnabled();
    if (key == GLFW_KEY_GRAVE_ACCENT) // || key == GLFW_KEY_ESCAPE)
    {
        if (action == GLFW_PRESS)
        {
            ZCmdConsole_SetInputEnabled(!bConsoleOn);
        }
        return YES;
    }

    //////////////////////////////////////////
    // Console overrides all
    if (ZCmdConsole_GetInputEnabled())
    {
        if (action == GLFW_PRESS && !bConsoleIgnore)
        {
            ZCmdConsole_WriteChar((char)key, g_bLeftShiftOn | g_bRightShiftOn);
        }
        return YES;
    }
    //////////////////////////////////////////
    // Platform 
	if ((key == GLFW_KEY_ESCAPE && !GetGameDef().bOverrideEscapeKey)
		|| key == GLFW_KEY_F8)
	{
        if (action == GLFW_PRESS)
        {
            ZEngine_BeginShutdown();
            Window_Shutdown();
        }
        return YES;
    }
    else if (key == GLFW_KEY_F7)
    {
        if (action == GLFW_PRESS)
        {
            Platform_DebugBreak();
        }
        return YES;
    }
    else if (key == GLFW_KEY_F6)
    {
        if (action == GLFW_PRESS)
        {
            ZR_Screenshot("screenshot.png");
        }
    }
    /*
    // console queuing test
    else if (key == GLFW_KEY_F5)
    {
        if (action == GLFW_PRESS)
        {
            ZCmdConsole_QueueCommand("map foo");
            ZCmdConsole_QueueCommand("set bar 7");
            ZCmdConsole_QueueCommand("help");
        }
    }*/
    return 0;
}

/**
 * Input mutex should already be locked when events are polled
 * so no sync is done in this callback
 */
static void key_callback(GLFWwindow *window, int glfwKey, int scancode, int action, int mods)
{
    // check for internal debug input handling
    if (handle_engine_key_event(window, glfwKey, scancode, action, mods) == YES)
    {
        return;
    }

    // read and pass on.
    zeInputCode keyCode = Win_GlfwToZEKey(glfwKey);
    if (keyCode == Z_INPUT_CODE_NULL)
    {
        // Unknown key. app will not have any idea what it is.
        return;
    }
    if (action == GLFW_PRESS)
    {
        //ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        //printf("WINDOW Writing key %d pressed\n", key);
        Sys_WriteInputEvent(&g_events, keyCode, 1, 1);
        //ZKeys_WriteEvent(&g_events, key, 1);
    }
    else if (action == GLFW_RELEASE)
    {
        //ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        //printf("WINDOW Writing key %d released\n", key);
        Sys_WriteInputEvent(&g_events, keyCode, 0, 1);
        //ZKeys_WriteEvent(&g_events, key, 0);
    }
}

static void window_close_callback(GLFWwindow *window)
{
    ZEngine_BeginShutdown();
}

static void mouse_position_callback(GLFWwindow* window, double posX, double posY)
{
    // printf("Mouse move %.3f, %.3f\n", posX, posY);
    // Sys_WriteInputEvent(&g_events, Z_INPUT_CODE_MOUSE_POS_X, 0, (f32)posX);
    // Sys_WriteInputEvent(&g_events, Z_INPUT_CODE_MOUSE_POS_Y, 0, (f32)posY);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

static void InitCallbacks(GLFWwindow *window)
{
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_position_callback);
}

//////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////
static GLFWmonitor *SelectMonitor()
{
    #if 1
    int count;
    GLFWmonitor **monitors = glfwGetMonitors(&count);
    int index = g_requestedMonitorIndex;
    if (index < 0) { index = 0; }
    if (index >= count) { index = count - 1; }
    printf("Found %d monitors, using %d\n", count, index);
    for (i32 i = 0; i < count; ++i)
    {
        i32 monitorX, monitorY;
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
        printf("Monitor %d pos: %d, %d\n", i, monitorX, monitorY);
    }
    return monitors[index];
    #endif
    #if 0
    return glfwGetPrimaryMonitor();
    #endif
}

//////////////////////////////////////////////////////////////////
// Create Window, gl context and renderer
//////////////////////////////////////////////////////////////////
ze_external zErrorCode SpawnWindow()
{
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

    // GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    GLFWmonitor *monitor = SelectMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    i32 scrWidth = mode->width;
    i32 scrHeight = mode->height;

    // set monitor size - necessary to get the ratio between
    // full screen size and window size for mouse input
    g_monitorSize[0] = scrWidth;
    g_monitorSize[1] = scrHeight;
    g_monitorAspect = (f32)scrWidth / (f32)scrHeight;

    if (g_bWindowed == YES)
    {
        // Resolution locked window
        // Disable decoration and set window size to desktop size
        // to have borderless fullscreen
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        scrWidth = g_pendingWidth;
        scrHeight = g_pendingHeight;
    }
    else
    {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }
    // record screen size
    g_windowSize[0] = scrWidth;
    g_windowSize[1] = scrHeight;
    g_windowAspect = (f32)scrWidth / (f32)scrHeight;
    printf("Aspect ratio %.3f\n", g_windowAspect);
    // Create!
    // Note: Passing any monitor here will enter dodgy horrible res-changing fullscreen mode
    // so just NULL NULL please.
    char* title = GetGameDef().windowTitle;
    if (title == NULL)
    {
        title = "Zealous Engine";
    }
    g_window = glfwCreateWindow(scrWidth, scrHeight, title, NULL, NULL);
	printf("Created window\n");
	// If in single frame mode, hide the window.
	if (GetSingleFrameMode())
	{
		glfwHideWindow(g_window);
	}	
	
    i32 monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);
    i32 x = monitorX;
    i32 y = monitorY;
    if (g_bWindowed)
    {
        x += 100;
        y += 100;
    }
    glfwSetWindowPos(g_window, x, y);

    if (!g_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // Init callbacks and events buffer
    InitCallbacks(g_window);
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

    #if 0
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
    #endif
    return ZE_ERROR_NONE;
}

ze_external zErrorCode ZWindow_Init()
{
    // register Exec_Windowed
    ZCmdConsole_RegisterInternalCommand(
        "windowed", "Set windowed, eg: windowed 1", Exec_Windowed);
    ZCmdConsole_QueueCommand("windowed 1");
    
    i32 index = ZCFG_FindParamIndex("-w", "--windowed", 0);
    g_bWindowed = (index != ZE_ERROR_BAD_INDEX) ? YES : NO;
    printf("Windowed: %d\n", g_bWindowed);

    g_events = Buf_FromMalloc(Platform_Alloc, KiloBytes(64));

    g_requestedMonitorIndex = ZCFG_FindIntParam("--monitor", "--monitor", 0);
	
	// read resolution overrides
	g_pendingWidth = ZCFG_FindIntParam("--width", "--width", g_pendingWidth);
	g_pendingHeight = ZCFG_FindIntParam("--height", "--height", g_pendingHeight);

    SpawnWindow();
    ZR_Init();
    return 0;
}

ze_external void Platform_PollEvents()
{
    g_events.Clear(NO);
    glfwPollEvents();
    // i32 bytesRead = g_events.Written();
    // if (bytesRead > 0)
    // {
    //     printf("Read %d bytes of platform events\n", bytesRead);
    // }
    // ...broadcast events here?
    f64 posX, posY;
    glfwGetCursorPos(g_window, &posX, &posY);
    f32 nX = (f32)posX / g_windowSize[0];
    f32 nY = (f32)posY / g_windowSize[1];
    nX = (nX * 2.f) - 1.f;
    nY = (nY * 2.f) - 1.f;
    Sys_WriteInputEvent(&g_events, Z_INPUT_CODE_MOUSE_POS_X, (i32)posX, nX);
    Sys_WriteInputEvent(&g_events, Z_INPUT_CODE_MOUSE_POS_Y, (i32)posY, nY);
    //ZEBuffer* buf = &g_events;
    BUF_BLOCK_BEGIN_READ((&g_events), header)
        if (header->type == ZE_SYS_EVENT_TYPE_INPUT)
        {
            ZInput_ReadEvent((SysInputEvent*)header);
        }
    BUF_BLOCK_END_READ
}

ze_external void Platform_SubmitFrame()
{
    glfwSwapBuffers(g_window);
}

ze_external void Platform_BeginDrawFrame()
{

}

ze_external void Platform_EndDrawFrame()
{
    glfwSwapBuffers(g_window);
}

ze_external zErrorCode ZWindow_Tick()
{
    return ZE_ERROR_NONE;
}

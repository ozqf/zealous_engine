
#include "ze_windows.h"

// glfw must come AFTER windows to avoid macro redefinition error
#include "../../../lib/glad/glad.h"
#include "../../../lib/glfw3_vc2015/glfw3.h"

// Version of Opengl that will be requested
const i32 MAJOR_VERSION_REQ = 3;
const i32 MINOR_VERSION_REQ = 3;

internal GLFWwindow *g_window;
internal i32 g_pendingWidth = 1600;
internal i32 g_pendingHeight = 900;
internal i32 g_bWindowed = FALSE;

internal i32 g_monitorSize[2];
internal f32 g_monitorAspect;

internal i32 g_windowSize[2];
internal f32 g_windowAspect;

// TODO: App thread can cause a 'GLFW is not initialised' error here
// by call into main thread during shutdown. Make sure App has closed before
// closing GLFW
static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

//////////////////////////////////////////////////////////////////
// GLFW Callbacks
//////////////////////////////////////////////////////////////////

internal void Window_StartShutdown(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    ZE_Shutdown();
}

/**
 * Handle inputs within this module. If they are handled, event is not
 * passed to app module.
 */
static i32 handle_window_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F8)
    {
        if (action == GLFW_PRESS)
        {
            Window_StartShutdown(window);
        }
        return YES;
    }
    return 0;
}

/**
 * Input mutex should already be locked when events are polled
 * so no sync is done in this callback
 */
static void key_callback(GLFWwindow *window, int glfwKey, int scancode, int action, int mods)
{
    if (handle_window_key(window, glfwKey, scancode, action, mods) == YES)
    {
        return;
    }
}

static void window_close_callback(GLFWwindow *window)
{
    Window_StartShutdown(window);
}

static void mouse_position_callback(GLFWwindow* window, double posX, double posY)
{

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
    #if 0
    int count;
    GLFWmonitor **monitors = glfwGetMonitors(&count);
    int index = 0;
    printf("Found %d monitors, using %d\n", count, index);
    for (i32 i = 0; i < count; ++i)
    {
        i32 monitorX, monitorY;
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
        printf("Monitor pos: %d, %d\n", monitorX, monitorY);
    }
    return monitors[index];
    #endif
    #if 1
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
    g_window = glfwCreateWindow(scrWidth, scrHeight, "Zealous Engine", NULL, NULL);

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
    i32 index = ZCFG_FindParamIndex("-w", "--windowed", 0);
    g_bWindowed = (index != ZE_ERROR_BAD_INDEX) ? YES : NO;
    printf("Windowed: %d\n", g_bWindowed);
    SpawnWindow();
    return 0;
}

ze_external void Platform_PollEvents()
{
    glfwPollEvents();
}

ze_external void Platform_Draw()
{

}

ze_external zErrorCode ZWindow_Tick()
{
    return ZE_ERROR_NONE;
}

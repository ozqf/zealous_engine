#ifndef ZE_WINDOW_CALLBACKS_H
#define ZE_WINDOW_CALLBACKS_H

#include "../ze_platform_events.h"

// TODO: App thread can cause a 'GLFW is not initialised' error here
// by call into main thread during shutdown. Make sure App has closed before
// closing GLFW
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_close_callback(GLFWwindow* window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    // TODO: Handle multithreading here for shutting down full app:
    /*
    if (g_app.isValid == YES)
    {
        g_app.AppImpl_BeginShutdown();
    }
    */
    // example where a close request could be denied...
    /*
    if (!time_to_close)
    {
        glfwSetWindowShouldClose(window, GLFW_FALSE);
    }
    */
}

/**
 * Input mutex should already be locked when events are polled
 * so no sync is done in this callback
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    if (action == GLFW_PRESS)
    {
        ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        ZKeys_WriteEvent(&g_events, key, 1);
    }
    else if (action == GLFW_RELEASE)
    {
        ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        ZKeys_WriteEvent(&g_events, key, 0);
    }
}

/**
 * Initialised GLFW event callbacks and event buffer
 */
static ErrorCode ZR_InitCallbacks(GLFWwindow* window)
{
	// Prepare buffer for platform events
    i32 eventBufferSize = KiloBytes(64);
    g_events = Buf_FromMalloc(malloc(eventBufferSize), eventBufferSize);

    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);

    if (glfwRawMouseMotionSupported())
    {
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
        
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return ZE_ERROR_NONE;
}

#endif // ZE_WINDOW_CALLBACKS_H
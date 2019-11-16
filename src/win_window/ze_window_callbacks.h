#ifndef ZE_WINDOW_CALLBACKS_H
#define ZE_WINDOW_CALLBACKS_H

//#include "../ze_platform_events.h"
#include "../sys_events.h"
#include "../ze_common/ze_input.h"
#include "win_map_glfw_input.h"
#include "win_console.h"

static i32 Win_IsCursorDisabled()
{
    return (g_bMouseCaptured && (g_consoleActive == NO));
}

static void Window_ApplyMouseState(GLFWwindow* window)
{
    i32 bCursorDisabled = Win_IsCursorDisabled();
    printf("Cursor disabled: %d\n", bCursorDisabled);
    if (bCursorDisabled)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

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

static i32 handle_window_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        if (action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        return YES;
    }
    // (not) tilde key console toggle:
    if (key == GLFW_KEY_GRAVE_ACCENT)
    {
        if (action == GLFW_PRESS)
        {
            g_consoleActive = !g_consoleActive;
            Console_Reset();
            Window_ApplyMouseState(window);
        }
        return YES;
    }
    if (g_consoleActive == YES)
    {
        if (action == GLFW_PRESS)
        {
            char* str = Console_AddChar((char)key);
            if (str != NULL)
            {
                // execute command
                printf("EXEC: %s\n", str);
            }
        }
        return YES;
    }
    return NO;
}

/**
 * Input mutex should already be locked when events are polled
 * so no sync is done in this callback
 */
static void key_callback(GLFWwindow* window, int glfwKey, int scancode, int action, int mods)
{
    if (handle_window_key(window, glfwKey, scancode, action, mods) == YES)
    {
        return;
    }
    
    zeInputCode keyCode = Win_GlfwToZEKey(glfwKey);
    if (keyCode == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    if (action == GLFW_PRESS)
    {
        //ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        //printf("WINDOW Writing key %d pressed\n", key);
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 1);
        //ZKeys_WriteEvent(&g_events, key, 1);
    }
    else if (action == GLFW_RELEASE)
    {
        //ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        //printf("WINDOW Writing key %d released\n", key);
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 0);
        //ZKeys_WriteEvent(&g_events, key, 0);
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    zeInputCode keyCode = Z_INPUT_CODE_NULL;
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT: keyCode = Z_INPUT_CODE_MOUSE_1; break;
        case GLFW_MOUSE_BUTTON_RIGHT: keyCode = Z_INPUT_CODE_MOUSE_2; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: keyCode = Z_INPUT_CODE_MOUSE_3; break;
    }
    if (keyCode == Z_INPUT_CODE_NULL) { return; }
    if (action == GLFW_PRESS)
    {
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 1);
    }
    else if (action == GLFW_RELEASE)
    {
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 0);
    }
}

/**
 * Initialised GLFW event callbacks and event buffer
 */
static ErrorCode ZR_InitCallbacks(GLFWwindow* window)
{
	// Prepare buffer for platform events
    i32 eventBufferSize = KiloBytes(64);
    //g_events = Buf_FromMalloc(malloc(eventBufferSize), eventBufferSize);

    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwGetCursorPos(window, &g_lastMouseSampleX, &g_lastMouseSampleY);
    Window_ApplyMouseState(window);
    return ZE_ERROR_NONE;
}

#endif // ZE_WINDOW_CALLBACKS_H
#ifndef ZE_WINDOW_CALLBACKS_H
#define ZE_WINDOW_CALLBACKS_H

//#include "../ze_platform_events.h"
#include "../sys_events.h"
#include "../../headers/common/ze_input.h"
#include "win_map_glfw_input.h"
#include "win_console.h"

static i32 Win_IsCursorDisabled()
{
    return (g_bMouseCaptured && (g_consoleActive == NO));
}

static void Window_ApplyMouseState(GLFWwindow* window)
{
    i32 bCursorDisabled = Win_IsCursorDisabled();
    if (bCursorDisabled)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
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
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        return YES;
    }
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
    if (key == GLFW_KEY_GRAVE_ACCENT) // || key == GLFW_KEY_ESCAPE)
    {
        if (action == GLFW_PRESS)
        {
            g_consoleActive = !g_consoleActive;
            printf("Console toggled to %d\n", g_consoleActive);
            Console_Reset();
            Window_ApplyMouseState(window);
        }
        return YES;
    }
    if (g_consoleActive == YES)
    {
        if (action == GLFW_PRESS && bConsoleIgnore == NO)
        {
            char* str = Console_AddChar((char)key, (g_bLeftShiftOn || g_bRightShiftOn));
            if (str != NULL)
            {
                printf(">> Window read cmd \"%s\"\n", str);
                //Window_EnqueueTextCommand(str);
                g_platform.EnqueueTextCommand(str);
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
	if (keyCode == Z_INPUT_CODE_NULL)
	{
		// Unknown key. app will not have any idea what it is.
		return;
	}
    // if (keyCode == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    // {
    //     glfwSetWindowShouldClose(window, GLFW_TRUE);
    //     return;
    // }
    if (action == GLFW_PRESS)
    {
        //ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        //printf("WINDOW Writing key %d pressed\n", key);
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 1, 1);
        //ZKeys_WriteEvent(&g_events, key, 1);
    }
    else if (action == GLFW_RELEASE)
    {
        //ZE_ASSERT(g_events.Space() >= sizeof(ze_key_event), "Events buffer overflow")
        //printf("WINDOW Writing key %d released\n", key);
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 0, 1);
        //ZKeys_WriteEvent(&g_events, key, 0);
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
    g_platform.EnqueueTextCommand("EXIT");
    //glfwSetWindowShouldClose(window, GLFW_TRUE);
    // TODO: Handle multithreading here for shutting down full app:
    /*
    if (g_app.sentinel == ZE_SENTINEL)
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

static void mouse_position_callback(GLFWwindow* window, double posX, double posY)
{
    // glfw gives pos as top left of window...
    g_mousePosX = posX;
    g_mousePosY = posY;
    // ...convert to -1 to 1 range
    g_mousePosNormalisedX = posX / g_windowSize.width;
    g_mousePosNormalisedX = (g_mousePosNormalisedX * 2.f) - 1.f;

    g_mousePosNormalisedY = posY / g_windowSize.height;
    g_mousePosNormalisedY = (g_mousePosNormalisedY * 2.f) - 1.f;
    g_mousePosNormalisedY *= -1.f;
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
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 1, 1);
    }
    else if (action == GLFW_RELEASE)
    {
        Sys_WriteInputEvent(&g_eventBuffer, keyCode, 0, 0);
    }
}

/**
 * Initialised GLFW event callbacks and event buffer
 */
static ErrorCode ZR_InitCallbacks(GLFWwindow* window)
{
	// Prepare buffer for platform events
    //i32 eventBufferSize = KiloBytes(64);
    //g_events = Buf_FromMalloc(malloc(eventBufferSize), eventBufferSize);

    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_position_callback);

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
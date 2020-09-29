#ifndef APP_UI_CPP
#define APP_UI_CPP

//#include "app.h"
#include "app_ui.h"
#include "../ui/zui.h"

#define APP_MENU_NONE 0
#define APP_MENU_ROOT 1

internal ze_platform_export g_platform;
internal ZUIScreen* g_menu = NULL;
internal Vec2 g_mousePos = {};
internal zeInputCode g_lastInputCode = Z_INPUT_CODE_NULL;

internal i32 g_currentMenu = APP_MENU_NONE;

// if true this key code should not be rebound
internal i32 IsInputCodeReserved(zeInputCode code)
{
    if (code == Z_INPUT_CODE_NULL
        || code == Z_INPUT_CODE_MOUSE_MOVE_X
        || code == Z_INPUT_CODE_MOUSE_MOVE_Y
        || code == Z_INPUT_CODE_MOUSE_POS_X
        || code == Z_INPUT_CODE_MOUSE_POS_Y
        || code == Z_INPUT_CODE_ESCAPE)
    {
        return YES;
    }
    return NO;
}

extern "C" void AppUI_Init(ze_platform_export platform)
{
	//APP_PRINT(64, "App UI Init\n");
    g_platform = platform;
    g_menu = ZUI_CreateScreen();
    g_menu->state = ZUI_SCREEN_STATE_ON;
    Colour off = COLOUR_WHITE;
    Colour on = COLOUR_BLACK; // COLOUR_YELLOW;
    ZUI_AddButton(g_menu, 0, -7, "START", off, on);
    ZUI_AddButton(g_menu, 0, -10, "OPTIONS", off, on);
    ZUI_AddButton(g_menu, 0, -13, "QUIT", off, on);
}

extern "C" i32 AppUI_IsActive()
{ return (g_currentMenu > APP_MENU_NONE); }

extern "C" void AppUI_Update()
{
	if (g_currentMenu < 0) { return; }
}

extern "C" ErrorCode AppUI_WriteFrame(ZRViewFrame* frame)
{
    if (g_currentMenu <= APP_MENU_NONE) { return ZE_ERROR_NONE; }
    switch (g_currentMenu)
    {
        case APP_MENU_ROOT:
        //printf("Try draw app menu (%d objects, %d state)\n", g_menu->numObjects, g_menu->state);
        //ILLEGAL_CODE_PATH
        ZUI_WriteScreenForRender(frame, g_menu, frame->list, frame->data);
        break;
    }
	return ZE_ERROR_NONE;
}

extern "C" i32 AppUI_ProcessInput(SysInputEvent ev)
{
    if (ev.inputID == APPUI_MENU_TOGGLE_KEY
        && ev.value != 0)
    {
        // process open/close
        if (AppUI_IsActive())
        {
            g_currentMenu = APP_MENU_NONE;
            return APPUI_INPUT_TOGGLED_OFF;
        }
        else
        {
            g_currentMenu = APP_MENU_ROOT;
            return APPUI_INPUT_TOGGLED_ON;
        }
    }
    // if we are not on drop out now
    if (!AppUI_IsActive()) { return APPUI_INPUT_UNHANDLED; }

    // test read for rebinding
    if (IsInputCodeReserved(ev.inputID) == NO
        && ev.inputID != g_lastInputCode)
    {
        printf("APPUI saw new input code %d\n", ev.inputID);
    }

    if (ev.inputID == Z_INPUT_CODE_MOUSE_1 && ev.value != 0)
    {
        if (g_menu->focusObjIndex < 0) { return APPUI_INPUT_UNHANDLED; }

        ZUIObject* obj = &g_menu->objects[g_menu->focusObjIndex];
        if (obj == NULL) { return APPUI_INPUT_HANDLED; }

        if (!ZE_CompareStrings(obj->label, "QUIT"))
        {
            g_platform.EnqueueTextCommand("QUIT");
        }
        if (!ZE_CompareStrings(obj->label, "START"))
        {
            g_platform.EnqueueTextCommand("START 1");
            g_currentMenu = APP_MENU_NONE;
            return APPUI_INPUT_TOGGLED_OFF;
        }
        return APPUI_INPUT_HANDLED;
    }
    if (ev.inputID == Z_INPUT_CODE_MOUSE_POS_X)
    {
        //printf("APPUI MPOS: %.3f\n", ev.value, ev.normalised);
        g_mousePos.x = ev.normalised;
        ZUI_UpdateMouseOverlap(g_menu, g_mousePos);
    }
    if (ev.inputID == Z_INPUT_CODE_MOUSE_POS_Y)
    {
        //printf("APPUI MPOS: %.3f\n", ev.value, ev.normalised);
        g_mousePos.y = ev.normalised;
        ZUI_UpdateMouseOverlap(g_menu, g_mousePos);
    }
    if (AppUI_IsActive())
    {
        return APPUI_INPUT_HANDLED;
    }
    // unhandled
    return APPUI_INPUT_UNHANDLED;
}

#endif // APP_UI_CPP
#ifndef APP_UI_CPP
#define APP_UI_CPP

//#include "app.h"
#include "app_ui.h"
#include "../ui/zui.h"

#define APP_MENU_NONE 0
#define APP_MENU_ROOT 1

internal ZUIScreen* g_menu = NULL;

internal i32 g_currentMenu = APP_MENU_NONE;

extern "C" void AppUI_Init()
{
	//APP_PRINT(64, "App UI Init\n");
    g_menu = ZUI_CreateScreen();
    g_menu->state = ZUI_SCREEN_STATE_ON;
    ZUI_AddButton(g_menu, 0, -7, "START", COLOUR_WHITE, COLOUR_YELLOW);
    ZUI_AddButton(g_menu, 0, -10, "OPTIONS", COLOUR_WHITE, COLOUR_YELLOW);
    ZUI_AddButton(g_menu, 0, -13, "QUIT", COLOUR_WHITE, COLOUR_YELLOW);
}

extern "C" i32 AppUI_IsActive()
{ return (g_currentMenu > APP_MENU_NONE); }

extern "C" void AppUI_Update(Vec2 mouseScreenPos)
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
                    && ev.value == 1)
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
    if (AppUI_IsActive())
    {
        return APPUI_INPUT_HANDLED;
    }
    // unhandled
    return APPUI_INPUT_UNHANDLED;
}

extern "C" void AppUI_OpenRoot()
{
    // printf("Open menu\n");
    // g_currentMenu = APP_MENU_ROOT;
}

#endif // APP_UI_CPP
#ifndef APP_UI_CPP
#define APP_UI_CPP

#include "app.h"
#include "../ui/zui.h"

#define ZE_WINDOW_NUM_CONSOLE_OBJECTS 64
internal ZUIObject g_mainMenuObjects[ZE_WINDOW_NUM_CONSOLE_OBJECTS];
internal ZUIScreen g_mainMenu;

internal ZUIScreen* g_menu = NULL;

internal i32 g_currentMenu = 0;

extern "C" void AppUI_Init()
{
	APP_PRINT(64, "App UI Init\n");
    g_menu = ZUI_CreateScreen();
    ZUI_AddButton(g_menu, 0, -7, "START", COLOUR_WHITE, COLOUR_YELLOW);
    ZUI_AddButton(g_menu, 0, -10, "OPTIONS", COLOUR_WHITE, COLOUR_YELLOW);
    ZUI_AddButton(g_menu, 0, -13, "QUIT", COLOUR_WHITE, COLOUR_YELLOW);
}

extern "C" void AppUI_Update(Vec2 mouseScreenPos)
{
	
}

extern "C" ErrorCode AppUI_WriteFrame(ZRViewFrame* frame)
{
    if (g_currentMenu < 0) { return ZE_ERROR_NONE; }
    switch (g_currentMenu)
    {
        case 0:
        //printf("Draw app menu (%d objects)\n", g_menu->numObjects);
        ZUI_WriteScreenForRender(
		    frame, g_menu, frame->list, frame->data);
        break;
    }
	return ZE_ERROR_NONE;
}

#endif // APP_UI_CPP
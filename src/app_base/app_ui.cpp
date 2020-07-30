#ifndef APP_UI_CPP
#define APP_UI_CPP

#include "app.h"
#include "../ui/zui.h"

#define ZE_WINDOW_NUM_CONSOLE_OBJECTS 2
internal ZUIObject g_mainMenuObjects[ZE_WINDOW_NUM_CONSOLE_OBJECTS];
internal ZUIScreen g_mainMenu;

extern "C" void AppUI_Init()
{
	APP_PRINT(64, "App UI Init\n");
	g_mainMenu = {};
	g_mainMenu.objects = g_mainMenuObjects;
	g_mainMenu.maxObjects = 2;
    g_mainMenu.state = 1;

    ZUIObject* obj = NULL;
    obj = &g_mainMenuObjects[g_mainMenu.numObjects++];
    *obj = {};
    obj->radiusInChars = { 32, 2 };
    obj->pos.x = 0;
    obj->pos.y = 0.5f;
    // label will be updated per frame anyway
    obj->label = "Testing McTest Face.";
    obj->charSize = ZR_CharScreenSizeDefault();
}

extern "C" void AppUI_Update(Vec2 mouseScreenPos)
{
	
}

extern "C" ErrorCode AppUI_WriteFrame(ZRViewFrame* frame)
{
	ZUI_WriteScreenForRender(
		frame, &g_mainMenu, frame->list, frame->data);
	return ZE_ERROR_NONE;
}

#endif // APP_UI_CPP
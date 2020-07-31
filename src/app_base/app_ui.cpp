#ifndef APP_UI_CPP
#define APP_UI_CPP

#include "app.h"
#include "../ui/zui.h"

#define ZE_WINDOW_NUM_CONSOLE_OBJECTS 64
internal ZUIObject g_mainMenuObjects[ZE_WINDOW_NUM_CONSOLE_OBJECTS];
internal ZUIScreen g_mainMenu;

internal i32 g_currentMenu = 0;

extern "C" void AppUI_Init()
{
	APP_PRINT(64, "App UI Init\n");
	g_mainMenu = {};
	g_mainMenu.objects = g_mainMenuObjects;
	g_mainMenu.maxObjects = 2;
    g_mainMenu.state = 1;

    f32 charSize = ZR_CharScreenSizeDefault();
    Point radius = { 32, 2 };
    ZUIObject* obj = NULL;

    // start button
    obj = &g_mainMenuObjects[g_mainMenu.numObjects++];
    *obj = {};
    obj->radiusInChars = radius;
    obj->pos.x = 0;
    obj->pos.y = -7 * charSize;
    // label will be updated per frame anyway
    obj->label = "START";
    obj->charSize = charSize;
    obj->fontColour = COLOUR_YELLOW;
    
    // start button
    obj = &g_mainMenuObjects[g_mainMenu.numObjects++];
    *obj = {};
    obj->radiusInChars = radius;
    obj->pos.x = 0;
    obj->pos.y = -9 * charSize;
    // label will be updated per frame anyway
    obj->label = "OPTIONS";
    obj->charSize = charSize;
    obj->fontColour = COLOUR_YELLOW;

    // start button
    obj = &g_mainMenuObjects[g_mainMenu.numObjects++];
    *obj = {};
    obj->radiusInChars = radius;
    obj->pos.x = 0;
    obj->pos.y = -11 * charSize;
    // label will be updated per frame anyway
    obj->label = "QUIT";
    obj->charSize = charSize;
    obj->fontColour = COLOUR_YELLOW;
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
        ZUI_WriteScreenForRender(
		    frame, &g_mainMenu, frame->list, frame->data);
        break;
    }
	return ZE_ERROR_NONE;
}

#endif // APP_UI_CPP
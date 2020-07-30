#ifndef APP_UI_CPP
#define APP_UI_CPP

#include "app.h"
#include "../ui/zui.h"

#define ZE_WINDOW_NUM_CONSOLE_OBJECTS
internal ZUIObject g_consoleUIObjs[2];
internal ZUIScreen g_consoleScene;

extern "C" void AppUI_Init()
{
	APP_PRINT(64, "App UI Init\n");
	g_consoleScene = {};
	g_consoleScene.objects = g_consoleUIObjs;
	g_consoleScene.maxObjects = 2;
    g_consoleScene.state = 1;

    ZUIObject* obj = NULL;
    obj = &g_consoleUIObjs[g_consoleScene.numObjects++];
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
		frame, &g_consoleScene, frame->list, frame->data);
	return ZE_ERROR_NONE;
}

#endif // APP_UI_CPP

#include "client_internal.h"
#include "../../ui/zui.h"

#define CL_MAX_HUD_OBJECTS 64
static ZUIObject g_objects[CL_MAX_HUD_OBJECTS];
static ZUIScreen g_hud;

static void CLUI_Init()
{
	g_hud = {};
	g_hud.objects = g_objects;
	g_hud.maxObjects = CL_MAX_HUD_OBJECTS;

	ZUIObject* obj = NULL;

	obj = &g_hud.objects[g_hud.numObjects++];
	obj->label = "START GAME";
    //g_testObj.radiusInChars.x = ZE_StrLen(g_testObj.label);
    obj->radiusInChars = { 32, 2 };
    obj->pos.y = -0.7f;
    obj->charSize = ZR_CharScreenSizeDefault();

	obj = &g_hud.objects[g_hud.numObjects++];
    obj->label = "QUIT";
    //g_testObj2.radiusInChars.x = ZE_StrLen(g_testObj.label);
    obj->radiusInChars = { 32, 2 };
    obj->pos.y = -0.8f;
    obj->charSize = ZR_CharScreenSizeDefault();
}

/**
 * Scenes added
 */
static void CLUI_AddDrawItems(ZRViewFrame* frame, ZEByteBuffer* list, ZEByteBuffer* data)
{
	ZUI_WriteScreenForRender(frame, &g_hud, list, data);
}

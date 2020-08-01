#ifndef ZUI_CPP
#define ZUI_CPP

#include "zui_draw.h"

static ZRAssetDB* g_db = NULL;

extern "C" void ZUI_Init(ZRAssetDB* db)
{
	g_db = db;
} 

extern "C" ZUIScreen* ZUI_CreateScreen()
{
    i32 maxObjects = 64;
    ZUIScreen* scr = (ZUIScreen*)malloc(sizeof(ZUIScreen));
    *scr = {};
    ZUIObject* objs = (ZUIObject*)malloc(sizeof(ZUIObject) * maxObjects);
    scr->objects = objs;
    scr->numObjects = 0;
    scr->maxObjects = maxObjects;
    scr->buttonRadius = { (i32)ZR_TEXT_SCREEN_LINE_COUNT / 2, 2 };
    scr->charSize = ZR_CharScreenSizeDefault();
    scr->state = 0;
    return scr;
}

extern "C" ZUIObject* ZUI_AddButton(
    ZUIScreen* scr, i32 gridX, i32 gridY, char* label, Colour offColour, Colour onColour)
{
    if (scr->numObjects >= scr->maxObjects)
    {
        return NULL;
    }
    ZUIObject* obj  = &scr->objects[scr->numObjects++];
    *obj = {};
    obj->type = ZUI_OBJ_TYPE_BUTTON;
    obj->gridPos = { gridX, gridY };
    obj->pos.x = gridX * scr->charSize;
    obj->pos.y = gridY * scr->charSize;
    obj->charSize = scr->charSize;
    obj->radiusInChars = scr->buttonRadius;
    obj->onColour = onColour;
    obj->offColour = offColour;
    obj->label = label;
    return obj;
}

#endif // ZUI_CPP
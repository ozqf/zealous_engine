#ifndef ZUI_CPP
#define ZUI_CPP

#include "zui_draw.h"

static i32 g_bInitialised = NO;
static ZRAssetDB* g_db = NULL;

extern "C" void ZUI_Init(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* db)
{
    if (g_bInitialised) { return; }
    g_bInitialised = YES;
    ZE_SetFatalError(fatalFunc);
	g_db = db;
} 

extern "C" ZUIScreen* ZUI_CreateScreen()
{
    i32 maxObjects = 64;
    ZUIScreen* scr = (ZUIScreen*)malloc(sizeof(ZUIScreen));
    *scr = {};
    ZUIObject* objs = (ZUIObject*)malloc(sizeof(ZUIObject) * maxObjects);
    scr->nextId = 1;
    scr->focusObjIndex = ZE_ERROR_BAD_INDEX;
    scr->objects = objs;
    scr->numObjects = 0;
    scr->maxObjects = maxObjects;
    scr->buttonRadius = { (i32)ZR_TEXT_SCREEN_LINE_COUNT / 4, 1 };
    scr->charSize = ZR_CharScreenSizeDefault();
    scr->state = 0;
    return scr;
}

extern "C" ZUIObject* ZUI_AddButton(
    ZUIScreen* scr, Point2 gridPos, char* label, Colour offColour, Colour onColour)
{
    if (scr->numObjects >= scr->maxObjects)
    {
        return NULL;
    }
    ZUIObject* obj  = &scr->objects[scr->numObjects++];
    *obj = {};
    obj->type = ZUI_OBJ_TYPE_BUTTON;
    obj->id = scr->nextId++;
    obj->gridPos = gridPos;
    obj->pos.x = gridPos.x * scr->charSize;
    obj->pos.y = gridPos.y * scr->charSize;
    obj->charSize = scr->charSize;
    obj->radiusInChars = scr->buttonRadius;
    obj->onColour = onColour;
    obj->offColour = offColour;
    obj->label = label;
    return obj;
}

extern "C" void ZUI_UpdateMouseOverlap(ZUIScreen* scr, Vec2 pos)
{
    scr->focusObjIndex = ZE_ERROR_BAD_INDEX;
    for (i32 i = 0; i < scr->numObjects; ++i)
    {
        ZUIObject* obj = &scr->objects[i];
        // reset to off
        obj->flags &= ~ZUI_OBJ_FLAG_HOVER;
        // check
        f32 charSize = obj->charSize;
        Vec2 objScrSize = 
        {
            ((f32)obj->radiusInChars.x * (f32)obj->charSize),
            ((f32)obj->radiusInChars.y * (f32)obj->charSize),
        };
        f32 minX = obj->pos.x - objScrSize.x;
        f32 maxX = obj->pos.x + objScrSize.x;
        // printf("ZUI MPOS %.3f, %.3f vs extents %.3f, %.3f char size %.3f\n",
        //     pos.x, pos.y, minX, maxX, obj->charSize);
        // printf("\tObj radius in chars - %d, %d\n",
        //     obj->radiusInChars.x, obj->radiusInChars.y);
        f32 minY = obj->pos.y - objScrSize.y;
        f32 maxY = obj->pos.y + objScrSize.y;
        if (pos.x > minX && pos.x < maxX && pos.y > minY && pos.y < maxY)
        {
            obj->flags |= ZUI_OBJ_FLAG_HOVER;
            scr->focusObjIndex = i;
        }
    }
}

#endif // ZUI_CPP
#ifndef ZUI_H
#define ZUI_H

#include "../ze_common/ze_common_full.h"

#define ZUI_SCREEN_STATE_OFF 0
#define ZUI_SCREEN_STATE_ON 1

#define ZUI_OBJ_TYPE_NONE 0
#define ZUI_OBJ_TYPE_BUTTON 1

#define ZUI_OBJ_FLAG_HOVER (1 << 0)

struct ZUIObject
{
	i32 id;
	i32 type;
	i32 flags;
	Point2 gridPos;
	Vec2 pos;
	// depth range is 1 (back) to -1 (front)
	f32 depth;
	Point2 radiusInChars;
	i32 state;
	char* label;
	f32 charSize;
	Colour onColour;
	Colour offColour;
	Colour bgColour;
};

struct ZUIScreen
{
	i32 id;
	ZUIObject* objects;
	i32 numObjects;
	i32 maxObjects;
	i32 state;
	f32 charSize;
	Point2 buttonRadius;
};

extern "C" void ZUI_Init(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* db);
extern "C" ZUIScreen* ZUI_CreateScreen();
//extern "C" i32 ZUI_WriteRenderTest(ZEByteBuffer* list, ZEByteBuffer* data);
extern "C" void ZUI_WriteScreenForRender(
	ZRViewFrame* frame, ZUIScreen* scr, ZEByteBuffer* list, ZEByteBuffer* data);
extern "C" void ZUI_UpdateMouseOverlap(ZUIScreen* scr, Vec2 pos);

extern "C" ZUIObject* ZUI_AddButton(
    ZUIScreen* scr, i32 gridX, i32 gridY, char* label, Colour offColour, Colour onColour);

#endif // ZUI_H
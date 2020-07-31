#ifndef ZUI_H
#define ZUI_H

#include "../ze_common/ze_common_full.h"

struct ZUIObject
{
	i32 id;
	i32 type;
	Vec2 pos;
	// depth range is 1 (back) to -1 (front)
	f32 depth;
	Point radiusInChars;
	i32 state;
	char* label;
	f32 charSize;
	Colour fontColour;
	Colour bgColour;
};

struct ZUIScreen
{
	i32 id;
	ZUIObject* objects;
	i32 numObjects;
	i32 maxObjects;
	i32 state;
};

extern "C" void ZUI_Init(ZRAssetDB* db);
extern "C" i32 ZUI_WriteRenderTest(ZEByteBuffer* list, ZEByteBuffer* data);
extern "C" void ZUI_WriteScreenForRender(
	ZRViewFrame* frame, ZUIScreen* scr, ZEByteBuffer* list, ZEByteBuffer* data);

#endif // ZUI_H
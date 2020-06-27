#ifndef ZUI_H
#define ZUI_H

#include "../ze_common/ze_common_full.h"

struct ZUITransform
{
	i32 id;
	Vec3 pos;
	Vec2 radius;
	char name[16];
};

struct ZUIButton
{
	i32 id;
};

extern "C" void ZUI();
extern "C" i32 ZUI_WriteRenderTest(ZEByteBuffer* list, ZEByteBuffer* data);

#endif // ZUI_H
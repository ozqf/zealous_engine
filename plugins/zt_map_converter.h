#ifndef ZT_MAP_CONVERTER_H
#define ZT_MAP_CONVERTER_H

#include "../headers/ze_common.h"

struct ZTMapOutput
{
	Vec3* verts;
	i32 numVerts;
	i32 maxVerts;
};

ze_external zErrorCode ZT_MapConvert(const char* mapText);
ze_external zErrorCode ZT_MapConvertTest(ZTMapOutput** output);
ze_external zErrorCode ZT_MapConvertFree(ZTMapOutput* output);

#endif // ZT_MAP_CONVERTER_H

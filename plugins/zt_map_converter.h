/*
TODO - work on or abandon .map file converted...?
Trenchbroom can output maps as .obj files so this is far less necessary
if we want to go that route for simple world geometry.
*/
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

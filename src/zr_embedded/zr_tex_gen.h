#ifndef ZR_TEX_GEN_H
#define ZR_TEX_GEN_H

#include "../ze_common/ze_common.h"

extern "C" i32 TexGen_BytesFor32BitImage(i32 width, i32 height)
{
	return (width * height) * sizeof(i32);
}

extern "C" i32 TexGen_BytesForBWImage(i32 width, i32 height)
{
	return (width * height) * sizeof(i32);
}

extern "C" void TexGen_SetRGBA(ColourU32* pixels, i32 width, i32 height, ColourU32 colour)
{
	i32 len = width * height;
	for (i32 i = 0; i < len; ++i)
	{
		pixels[i] = colour;
	}
}

#endif // ZR_TEX_GEN_H
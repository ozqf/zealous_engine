#ifndef ZR_TEX_GEN_H
#define ZR_TEX_GEN_H

#include "../ze_common/ze_common.h"

extern "C" i32 TexGen_BytesFor32BitImage(i32 width, i32 height)
{
	return (width * height) * sizeof(i32);
}

extern "C" i32 TexGen_BytesForBWImage(i32 width, i32 height)
{
	return (width / 8) * height;
}


//////////////////////////////////////////////////
// 32bit bitmap draw
//////////////////////////////////////////////////
extern "C" void TexGen_SetRGBA(ColourU32* pixels, i32 width, i32 height, ColourU32 colour)
{
	i32 len = width * height;
	for (i32 i = 0; i < len; ++i)
	{
		pixels[i] = colour;
	}
}


extern "C" void TexGen_FillRect(
	ColourU32* pixels, i32 texWidth, i32 texHeight, Point topLeft, Point size, ColourU32 colour)
{
	i32 endX = topLeft.x + size.x;
	i32 endY = topLeft.y + size.y;
	if (topLeft.x < 0) { topLeft.x = 0; }
	if (topLeft.y < 0) { topLeft.y = 0; }
	if (endX > texWidth) { endX = texWidth; }
	if (endY > texHeight) { endY = texHeight; }
	//printf("Fill from %d, %d for %d, %d pixels\n", )
	for (i32 y = topLeft.y; y < endY; ++y)
	{
		for (i32 x = topLeft.x; x < endX; ++x)
		{
			i32 i = (x + (y * texWidth));
			pixels[i] = colour;
		}
	}
}

/**
 * move down the texture drawing lines across it at intervals
 */
extern "C" void TexGen_PaintHorizontalLines(
	ColourU32* pixels, i32 texWidth, i32 texHeight, ColourU32 colour)
{
	i32 numLines = 5;
	i32 step = texHeight / 5;
	ILLEGAL_CODE_PATH
}

//////////////////////////////////////////////////
// Black & White
//////////////////////////////////////////////////
extern "C" i32 TexGen_EncodeBW(
	u8* dest, const i32 destSize, ColourU32* pixels, const i32 w, const i32 h)
{
	i32 blocksWritten = 0;
	i32 requiredSize = TexGen_BytesForBWImage(w, h);
	if (requiredSize != destSize)
	{
		return ZE_ERROR_NO_SPACE;
	}
	// width must be divisible by 8
	if (w % 8 != 0)
	{
		return ZE_ERROR_BAD_SIZE;
	}
	// step pixels in 8 by 8 blocks
	i32 numPixels = w * h;
	for (i32 i = 0; i < numPixels; i += 8)
	{
		u8* block = &dest[i / 8];
		*block = 0;
		// step eight colours
		for (i32 bit = 0; bit < 8; ++bit)
		{
			ColourU32 colour = pixels[i + bit];
			if (colour.r > 0 || colour.g > 0 || colour.b > 0)
			{
				*block |= (1 << bit);
			}
		}
		blocksWritten++;
	}
	if (blocksWritten != requiredSize)
	{
		printf("ERROR - Unexpected block count\n");
	}
	return ZE_ERROR_NONE;
}

extern "C" i32 TexGen_DecodeBW(
	u8* source, const i32 sourceSize, ColourU32* target, const i32 w, const i32 h, ColourU32 solid, ColourU32 empty)
{
	for (i32 i = 0; i < sourceSize; ++i)
	{
		u8 block = source[i];
		for (i32 bit = 0; bit < 8; ++bit)
		{
			ColourU32* colour = &target[(i * 8) + bit];
			if (block & (1 << bit))
			{
				*colour = solid;
			}
			else
			{
				*colour = empty;
			}
		}
	}
	return ZE_ERROR_NONE;
}

#endif // ZR_TEX_GEN_H
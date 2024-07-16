#include "../../internal_headers/zengine_internal.h"

#define STBI_NO_STDIO

//#define STBI_MALLOC(sz)           malloc(sz)
//#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
//#define STBI_FREE(p)              free(p)

#define STBI_MALLOC(sz)			Platform_Alloc(sz)
#define STBI_REALLOC(p,newsz)	Platform_Realloc(p, newsz)
#define STBI_FREE(p)			Platform_Free(p)

#define STB_IMAGE_IMPLEMENTATION

#include "../../../lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include "../../../lib/stb_image_write.h"

ze_internal void CheckAllBlack(i32 width, i32 height, const void *rgbPixels)
{
	u8* pixels = (u8*)rgbPixels;
	i32 numPixels = width * height;
	// step 3 for 3 channels
	for (i32 i = 0; i < numPixels; i += 3)
	{
		u8* cursor = &pixels[i];
		u8 r = cursor[i + 0];
		u8 g = cursor[i + 1];
		u8 b = cursor[i + 2];
		if (r != 0 || g != 0 || b != 0)
		{
			return;
		}
	}
	printf("WARNING: Screenshot is all black!\n");
}

ze_external ZRTexture* ZAssets_LoadPngTextureFromFile(const char* path)
{
	// TODO - so this involves loading the file three times:
	// staging, stbi and finally the asset db.

	// stage
	ZEBuffer buffer = Platform_StageFile(path);

	// read to PNG
	i32 width;
	i32 height;
	i32 comp;
	i32 reqComp = 0;
	u8* data = stbi_load_from_memory(
		(u8*)buffer.start,
		(i32)(buffer.cursor - buffer.start),
		&width,
		&height,
		&comp,
		reqComp);
	
	ZRTexture* tex = ZAssets_AllocTex(width, height, (char*)path);
	
	// slam those pixels in
	i32 numPixels = width * height;
	i32 i = 0;
	
	ColourU32* source = (ColourU32*)data;
	ColourU32* dest = tex->data;
	while (i < numPixels)
	{
		dest[numPixels - i] = source[i];
		i++;
	}

	// cleanup
	STBI_FREE(data);
	Platform_Free(buffer.start);
	return tex;
}

ze_external void ZAssets_SaveImage(
	const char *fileName, i32 width, i32 height, const void *rgbPixels)
{
	CheckAllBlack(width, height, rgbPixels);
	stbi_flip_vertically_on_write(YES);
	stbi_write_png(fileName, width, height, 3, rgbPixels, width * 3);
	printf("Wrote %s\n", fileName);
}

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

ze_external void ZAssets_SaveImage(
	const char *fileName, i32 width, i32 height, const void *rgbPixels)
{
	stbi_flip_vertically_on_write(YES);
	stbi_write_png(fileName, width, height, 3, rgbPixels, width * 3);
}

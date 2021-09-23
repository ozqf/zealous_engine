#include "ze_opengl_internal.h"

ze_external i32 Vec4Tex_IsPosSafe(ZRVec4Texture* tex, i32 x, i32 y)
{
	if (tex == NULL) { return NO; }
	return (x >= 0
		&& x < tex->width
		&& y >= 0
		&& y < tex->height);
}

ze_external zErrorCode Vec4Tex_SetAt(ZRVec4Texture* tex, i32 x, i32 y, Vec4 v)
{
	if (!Vec4Tex_IsPosSafe(tex, x, y)) { return ZE_ERROR_OUT_OF_BOUNDS; }
	i32 i = ZE_2D_INDEX(x, y, tex->width);
	tex->data[i] = v;
	return ZE_ERROR_NONE;
}

ze_external void Vec4Tex_SetAll(ZRVec4Texture* tex, Vec4 value)
{
	i32 totalPixels = tex->width * tex->height;
	for (i32 i = 0; i < totalPixels; ++i)
	{
		tex->data[i] = value;
	}
}

ze_external ZRVec4Texture* Vec4Tex_Alloc(i32 width, i32 height)
{
	zeSize totalPixels = width * height;
	zeSize pixelBytes = totalPixels * sizeof(Vec4);
	zeSize totalBytes = sizeof(ZRVec4Texture) + pixelBytes;
	
	u8* mem = (u8*)Platform_Alloc(totalBytes);
	ZRVec4Texture* tex = (ZRVec4Texture*)mem;
	*tex = {};
	tex->data = (Vec4*)(mem + sizeof(ZRVec4Texture));
	tex->width = width;
	tex->height = height;
	tex->bIsDirty = YES;
	return tex;
}

/*
ze_external void Vec4Tex_CreateVec4DataTexture(i32 width, i32 height)
{
	GLuint g_dataTextureHandle;
	glGenTextures(1, &g_dataTextureHandle);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_2D, g_dataTextureHandle);
    CHECK_GL_ERR
	
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
		width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    CHECK_GL_ERR
}
*/

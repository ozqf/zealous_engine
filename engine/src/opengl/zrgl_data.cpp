#include "ze_opengl_internal.h"

ze_external i32 ZEGrid2D_IsPosSafe(ZEGrid2D* grid, i32 x, i32 y)
{
	if (grid == NULL) { return NO; }
	return (x >= 0
		&& x < grid->width
		&& y >= 0
		&& y < grid->height);
}

ze_external zErrorCode Vec4Tex_SetAt(ZRVec4Texture* tex, i32 x, i32 y, Vec4 v)
{
	if (!ZEGrid2D_IsPosSafe(&tex->header, x, y)) { return ZE_ERROR_OUT_OF_BOUNDS; }
	i32 i = ZE_2D_INDEX(x, y, tex->header.width);
	tex->data[i] = v;
	return ZE_ERROR_NONE;
}

ze_external void Vec4Tex_SetAll(ZRVec4Texture* tex, Vec4 value)
{
	i32 totalPixels = tex->header.width * tex->header.height;
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
	tex->header.width = width;
	tex->header.height = height;
	tex->header.bIsDirty = YES;
	return tex;
}

ze_external ZRU16Texture* U16Tex_Alloc(i32 width, i32 height)
{
	zeSize totalPixels = width * height;
	zeSize pixelBytes = totalPixels * sizeof(u16);
	zeSize totalBytes = sizeof(ZRU16Texture) + pixelBytes;
	
	u8* mem = (u8*)Platform_Alloc(totalBytes);
	ZRU16Texture* tex = (ZRU16Texture*)mem;
	*tex = {};
	tex->data = (u16*)(mem + sizeof(ZRU16Texture));
	tex->header.width = width;
	tex->header.height = height;
	tex->header.bIsDirty = YES;
	return tex;
}

/*
Returns handle.
Data can be null
*/
ze_external GLuint Vec4Tex_Register(ZRVec4Texture* tex)
{
	GLuint dataTextureHandle;
	glGenTextures(1, &dataTextureHandle);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_2D, dataTextureHandle);
    CHECK_GL_ERR
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
		tex->header.width, tex->header.height, 0, GL_RGBA, GL_FLOAT, tex->data);
    CHECK_GL_ERR
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERR
	return dataTextureHandle;
}

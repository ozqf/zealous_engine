#include "ze_opengl_internal.h"

struct ZRVec4Texture
{
	GLuint handle;
	i32 width;
	i32 height;
	Vec4* data;
};

ze_external void Vec4Tex_SetAll(ZRVec4Texture* tex, Vec4 value)
{
	i32 totalPixels = tex->width * tex->height;
	for (i32 i = 0; i < totalPixels; ++i)
	{
		tex->data[i] = value;
	}
}

ze_external void ZRGL_CreateVec4DataTexture(i32 width, i32 height)
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

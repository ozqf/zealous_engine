#ifndef ZRGL_PREFABS
#define ZRGL_PREFABS

#include "zrgl_internal.h"

// Asset libraries:
//#define STB_IMAGE_IMPLEMENTATION
//#include "../../lib/stb_image.h"

#include "../../lib/openfbx/ofbx.h"

// Returns opengl handle or 0 if failed
// Length == pixels wide and high, each pixel being 4 floats
// As 1D, can't be big enough...?
static ZRDataTexture ZRGL_CreateDataTexture2D(
    ZRPlatform* plat, i32 length)
{
    ZRDataTexture tex = {};
    tex.bIs1D = NO;
    tex.width = length;
    tex.height = length;
    // Calc bytes required
    tex.numBytes = sizeof(Vec4) * (tex.width * tex.height);
    tex.mem = (Vec4*)COM_Malloc(&g_mallocs, tex.numBytes, ZRGL_ALLOC_TAG_DATA_TEXTURE, "2d data tex");
    //tex.mem = (Vec4*)plat->Allocate(tex.numBytes);
    ZE_ASSERT(tex.mem != NULL, "Failed to allocate memory for texture\n");
    ZE_SET_ZERO((u8*)tex.mem, tex.numBytes);
    glGenTextures(1, &tex.handle);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    CHECK_GL_ERR
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
        tex.width, tex.height, 0, GL_RGBA, GL_FLOAT, tex.mem);
    CHECK_GL_ERR
    return tex;
}

static ZRDataTexture ZRGL_CreateDataTexture1D(
    ZRPlatform* plat, i32 length)
{
    ZRDataTexture tex = {};
    tex.bIs1D = YES;
    tex.width = length;
    tex.height = 1;
    // Calc bytes required
    tex.numBytes = sizeof(Vec4) * tex.width;
    tex.mem = (Vec4*)COM_Malloc(&g_mallocs, tex.numBytes, ZRGL_ALLOC_TAG_DATA_TEXTURE, "1d data tex");
    ZE_ASSERT(tex.mem != NULL, "Failed to allocate memory for texture\n");
    ZE_SET_ZERO((u8*)tex.mem, tex.numBytes);
    glGenTextures(1, &tex.handle);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_1D, tex.handle);
    CHECK_GL_ERR
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F,
        tex.width, 0, GL_RGBA, GL_FLOAT, tex.mem);
    CHECK_GL_ERR
    return tex;
}
#if 0
static u32 ZRGL_LoadCubeMap(
    char** paths, // Must have a length of 6!
    i32 bVerbose)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    CHECK_GL_ERR
    printf("Creating skybox %d\n", textureID);
    /*
    GL_TEXTURE_CUBE_MAP_POSITIVE_X 	Right
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	Left
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	Top
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	Bottom
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	Back
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	Front
    */
    //
    int width, height;//, nrChannels;
    unsigned char *data;
    for(GLuint i = 0; i < 6; i++)
    {
        data = ZRGL_LoadTextureToHeap(paths[i], NO, &width, &height, NO);
        //data = stbi_load(paths[i], &width, &height, &nrChannels, STBI_rgb_alpha);
        if (data != NULL)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            CHECK_GL_ERR
            //printf("Loaded side %d: %s (%d / %d)\n", i, paths[i], width, height);
        }
        else
        {
            printf("  Failed to tex %s\n", paths[i]);
        }
        //stbi_image_free(data);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    CHECK_GL_ERR
    return textureID;
}
#endif

#endif // ZRGL_PREFABS
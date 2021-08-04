#ifndef ZE_OPENGL_INTERNAL_H
#define ZE_OPENGL_INTERNAL_H

#include "../../internal_headers/zengine_internal.h"

// third party
#include "../../../lib/glad/glad.h"
#include "../../../lib/glfw3_vc2015/glfw3.h"

// Finding this makes it easier to hunt down specific error locations
// vs using opengl's error callback...
#define CHECK_GL_ERR                                                                                           \
    {                                                                                                          \
        GLenum checkGlErrVal;                                                                                  \
        while ((checkGlErrVal = glGetError()) != GL_NO_ERROR)                                                  \
        {                                                                                                      \
            char *checkGlErrType = "Unknown";                                                                  \
            switch (checkGlErrVal)                                                                             \
            {                                                                                                  \
            case 0x500:                                                                                        \
                checkGlErrType = "GL_INVALID_ENUM";                                                            \
                break;                                                                                         \
            case 0x501:                                                                                        \
                checkGlErrType = "GL_INVALID_VALUE";                                                           \
                break;                                                                                         \
            case 0x502:                                                                                        \
                checkGlErrType = "GL_INVALID_OPERATION";                                                       \
                break;                                                                                         \
            }                                                                                                  \
            printf("GL Error 0x%X (%s) at %s line %d\n", checkGlErrVal, checkGlErrType, __FILE__, (__LINE__)); \
        }                                                                                                      \
    }

#define ZRGL_DATA_ATTRIB_VERTICES 0
#define ZRGL_DATA_ATTRIB_UVS 1
#define ZRGL_DATA_ATTRIB_NORMALS 2

///////////////////////////////////////////////////////
// Internal datatypes
///////////////////////////////////////////////////////
/**
 * Asset handles required to execute a draw call
 */
struct ZRMeshHandles
{
    i32 vao;
    i32 vbo;
    i32 vertexCount;
    i32 totalVBOBytes;
    // all data before this point is static mesh geometry
    i32 instanceDataOffset;
    // Capacity for instances left behind static mesh data
    i32 maxInstances;
};

struct ZRGLHandles
{
    i32 assetId;
    i32 assetType;
    union
    {
        ZRMeshHandles meshHandles;
        u32 textureHandle;
    } data;
};

struct ZRShader
{
    i32 handle;      // considered invalid if this is 0
    i32 drawObjType; // considered invalid if this is 0
    i32 bBatchable;
    char *name;
};

/**
 * handles for executing a draw call
 */
struct ZRMeshDrawHandles
{
    i32 vao;
    i32 vertCount;
    i32 diffuseHandle;
    i32 emissiveHandle;
};

///////////////////////////////////////////////////////
// Shader setup
///////////////////////////////////////////////////////
ze_external void ZR_SetProg1i(GLuint prog, char *uniform, GLint value);
ze_external void ZR_SetProg1f(GLuint prog, char *uniform, GLfloat value);
ze_external void ZR_SetProgVec3f(GLuint prog, char *uniform, Vec3 vec);
ze_external void ZR_SetProgVec4f(GLuint prog, char *uniform, Vec4 vec);
ze_external void ZR_SetProgM4x4(GLuint prog, char *uniform, f32 *matrix);
ze_external void ZR_PrepareTextureUnit1D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char *uniformName,
    GLint texture,
    GLint sampler);
ze_external void ZR_PrepareTextureUnit2D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char *uniformName,
    GLint texture,
    GLint sampler);
ze_external void ZR_PrepareTextureUnitCubeMap(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char *uniformName,
    GLint texture,
    i32 samplerType);
ze_external ErrorCode ZRGL_CreateProgram(
    const char *vertexShader,
    const char *fragmentShader,
    char *shaderName,
    const i32 drawObjType,
    const i32 bIsBatchable,
    ZRShader *result);
ze_external void ZRGL_PrintShaderCompileLog(GLuint shaderId);

///////////////////////////////////////////////////////
// Uploading
///////////////////////////////////////////////////////
ze_external void ZRGL_UploadTexture(u8 *pixels, i32 width, i32 height, u32 *handle);
ze_external void ZRGL_UploadMesh(ZRMeshData *data, ZRMeshHandles *result, u32 flags);
ze_external void ZRGL_UploaderInit();
ze_external u32 ZRGL_GetTextureHandle(i32 assetId);
ze_external zErrorCode ZRGL_InitShaders();

///////////////////////////////////////////////////////
// Drawing
///////////////////////////////////////////////////////
ze_external void ZRDraw_SpriteBatch(
    ZRDrawCmdSpriteBatch *batch, M4x4 *view, M4x4 *projection);

///////////////////////////////////////////////////////
// Debug
///////////////////////////////////////////////////////
ze_external void ZRGL_Debug_Init();
ze_external void OpenglTest_DrawScreenSpaceQuad();
ze_external void ZRGL_Debug_DrawCubeTest();
ze_external void ZRGL_Debug_DrawWorldCubeTest();
ze_external void ZRGL_Debug_DrawWorldSprites();

#endif // ZE_OPENGL_INTERNAL_H

#ifndef ZE_OPENGL_INTERNAL_H
#define ZE_OPENGL_INTERNAL_H

#include "../../internal_headers/zengine_internal.h"

#include "shaders/zrgl_embedded_shaders.h"

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

#define ZR_SHADER_FLAG_NO_DEPTH (1 << 0)
#define ZR_SHADER_FLAG_BLEND (1 << 1)

struct ZRShader
{
    i32 handle;      // considered invalid if this is 0
    i32 drawObjType; // considered invalid if this is 0
    i32 flags;
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
ze_external void ZR_PrepareShader(ZRShader* shader);
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
    GLuint sampler);
ze_external void ZR_PrepareTextureUnit2D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char *uniformName,
    GLint texture,
    GLuint sampler);
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
    ZRShader *result);
ze_external void ZRGL_PrintShaderCompileLog(GLuint shaderId);

///////////////////////////////////////////////////////
// Uploading
///////////////////////////////////////////////////////
ze_external void ZRGL_UploadTexture(u8 *pixels, i32 width, i32 height, u32 *handle);
ze_external void ZRGL_UploadMesh(ZRMeshData *data, ZRMeshHandles *result, u32 flags);
ze_external void ZRGL_UploaderInit();
ze_external zErrorCode ZRGL_InitShaders();

// handles
ze_external u32 ZRGL_GetTextureHandle(i32 assetId);
ze_external ZRMeshHandles *ZRGL_GetMeshHandles(i32 assetId);
ze_external ZRGLHandles* ZRGL_GetHandleData(i32 assetId);

///////////////////////////////////////////////////////
// Drawing
///////////////////////////////////////////////////////
ze_external void ZRDraw_SpriteBatch(
    ZRDrawCmdSpriteBatch *batch, M4x4 *view, M4x4 *projection);
ze_external void ZRGL_DrawMesh(ZRDrawCmdMesh* meshCmd, M4x4* view, M4x4* projection);
ze_external void ZRGL_DrawDebugLines(ZRDrawCmdDebugLines* cmd, M4x4* view, M4x4* projection);

///////////////////////////////////////////////////////
// Data texture
///////////////////////////////////////////////////////
ze_external ZRVec4Texture* Vec4Tex_Alloc(i32 width, i32 height);
ze_external ZRU16Texture* U16Tex_Alloc(i32 width, i32 height);
ze_external void Vec4Tex_SetAll(ZRVec4Texture* tex, Vec4 value);
ze_external zErrorCode Vec4Tex_SetAt(ZRVec4Texture* tex, i32 x, i32 y, Vec4 v);
ze_external i32 ZEGrid2D_IsPosSafe(ZEGrid2D* grid, i32 x, i32 y);

ze_external GLuint Vec4Tex_Register(ZRVec4Texture* tex);

///////////////////////////////////////////////////////
// Debug
///////////////////////////////////////////////////////
ze_external void ZRGL_Debug_Init();
ze_external void ZRGL_SandboxRunTest(i32 mode);

#endif // ZE_OPENGL_INTERNAL_H

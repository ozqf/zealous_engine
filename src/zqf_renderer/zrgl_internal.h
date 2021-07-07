#ifndef ZQF_GL_INTERNAL_H
#define ZQF_GL_INTERNAL_H
/**
 * Header for internal components of renderer
 */
#include "zrgl.h"

#define ZRGL_BAD_HANDLE_ID 0

#define ZRGL_ALLOC_TAG_DATA_TEXTURE 1

#define ZRGL_DEBUG_BIT_SHOWGBUFFER (1 << 0)
#define ZRGL_DEBUG_BIT_SHOWSTATS (1 << 1)

// Advance buffer data in shader by each vertex being rendered
#define ZRGL_DIVISOR_PER_VERTEX 0
// Advance buffer data in shader by each instance being rendered
#define ZRGL_DIVISOR_PER_INSTANCE 1

#define ZRGL_DATA_ATTRIB_VERTICES 0
#define ZRGL_DATA_ATTRIB_UVS 1
#define ZRGL_DATA_ATTRIB_NORMALS 2

#define ZRGL_DATA_ATTRIB_MODEL_VIEW_COLUMN_0 3
#define ZRGL_DATA_ATTRIB_MODEL_VIEW_COLUMN_1 4
#define ZRGL_DATA_ATTRIB_MODEL_VIEW_COLUMN_2 5
#define ZRGL_DATA_ATTRIB_MODEL_VIEW_COLUMN_3 6

#define ZR_MAX_PROGRAMS 128

#define ZR_SHADER_TYPE_NONE -1
#define ZR_SHADER_TYPE_FALLBACK 0
#define ZR_SHADER_TYPE_TEST 1
#define ZR_SHADER_TYPE_BLOCK_COLOUR 2
#define ZR_SHADER_TYPE_BATCHED 3
#define ZR_SHADER_TYPE_TEXT 4
#define ZR_SHADER_TYPE_SKYBOX 5
#define ZR_SHADER_TYPE_BLOCK_COLOUR_BATCHED 6
#define ZR_SHADER_TYPE_SHADOW_MAP 7
#define ZR_SHADER_TYPE_SHADOW_MAP_DEBUG 8
#define ZR_SHADER_TYPE_BUILD_GBUFFER 9
#define ZR_SHADER_TYPE_COMBINE_GBUFFER 11
#define ZR_SHADER_TYPE_GBUFFER_LIGHT_DIRECT 12
#define ZR_SHADER_TYPE_GBUFFER_LIGHT_POINT 13
#define ZR_SHADER_TYPE_GBUFFER_LIGHT_VOLUME 14
#define ZR_SHADER_TYPE_LAST__ 14

////////////////////////////////////////////////////
// For drawing quick bitmap text:
////////////////////////////////////////////////////
// 6 verts * 3 floats * 4 bytes
#define ZRGL_BYTES_FOR_QUAD_VERTS 72
#define ZRGL_BYTES_FOR_QUAD_UVS 48
// verts/uvs/normals - 72/48/72
#define ZRGL_BYTES_FOR_QUAD_TOTAL 192

#define ZRGL_ASCI_CHARSET_CHARS_WIDE 16

#include <stdio.h>

#include "../../lib/glad/glad.h"
#include "../../headers/common/ze_common_full.h"
#include "../../lib/shaders.h"

// External interface
#include "../../headers/zqf_renderer.h"

// embedded assets
#include "../zr_embedded/zr_embedded.h"
// loaded assets
//#include "zr_asset_db.h"

#include "opengl_utils.h"
#include "../../headers/common/ze_buf_block.h"

///////////////////////////////////////////////////////
// Internal datatypes
///////////////////////////////////////////////////////

struct ZRShader
{
    i32 handle; // considered invalid if this is 0
    i32 drawObjType; // considered invalid if this is 0
    i32 bBatchable;
    char* name;
};

struct ZRFrameBuffer
{
    GLuint fbo;
    GLuint colourTex;
    GLuint depthTex;
    GLuint depthRenderBuf;
};

struct ZR_DeferredStats
{
    f32 geometryMS;
    f32 lightingMS;
    i32 numLights;
};

struct ZRGBuffer
{
    i32 bIsValid;
    i32 width;
    i32 height;
    GLuint fbo;
    GLuint positionTex;
    GLuint normalTex;
    GLuint colourTex;
    GLuint emissionTex;

    GLuint depthRenderBuf;
};

struct ZRShaderHandle
{
    GLint programHandle;
    i32 batchingFunction;
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

struct ZRShadowCaster
{
    Transform worldT;
    M4x4 projection;
    M4x4 view;
};

struct ZRSpriteAtlas
{
    i32 id;
    i32 textureHandle;
    i32 numSprites;
    // control scaling of sprites to screen.
    i32 pixPerUnit;
};

struct ZRSprite
{
    i32 id;
    i32 atlasId;
    Vec2 uvMin;
    Vec2 uvMax;
};

////////////////////////////////////////////////////////////
// internal globals
////////////////////////////////////////////////////////////

struct ZRGPUSpecs
{
    GLint maxTextureSize; // GL_MAX_TEXTURE_SIZE
    GLint current_mem_kb; // GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
    GLint maxCombinedTextureUnits; // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
    GLint maxVertexAttribs; // GL_MAX_VERTEX_ATTRIBS

    struct
    {
        GLint maxUniformBlockSize; // GL_MAX_UNIFORM_BLOCK_SIZE
        GLint uniformBufferOffsetAlignment; // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT
        //GLint maxVertexUniformBlocks; // GL_MAX_VERTEX_UNIFORM_BLOCKS
        //GLint maxGeometryUniformBlocks; // GL_MAX_VERTEX_UNIFORM_BLOCKS
        //GLint maxFragmentUniformBLocks; // GL_MAX_FRAGMENT_UNIFORM_BLOCKS
    } uniformBlocks;
};

// Note 1: A full RGBA32F texture at 1024x1024 is 16meg and really slow to upload!
//  512 however seems okay on my old GTX 970...so to do async texture uploads?
// Note 2: 1D textures are still limited to the same side length!
// so a 1D texture cannot be 1024 * 1024 wide!
#define ZQF_GL_DATA_TEXTURE_WIDTH 512

////////////////////////////////////////////////////////////
// internal functions
////////////////////////////////////////////////////////////
extern "C" ErrorCode ZRGL_CreateProgram(
    const char *vertexShader,
    const char *fragmentShader,
    char* shaderName,
    const i32 drawObjType,
    const i32 bIsBatchable,
    ZRShader* result);

extern "C" void ZR_SetProg1i(GLuint prog, char* uniform, GLint value);
extern "C" void ZR_SetProg1f(GLuint prog, char* uniform, GLfloat value);
extern "C" void ZR_SetProgVec3f(GLuint prog, char* uniform, Vec3 vec);
extern "C" void ZR_SetProgVec4f(GLuint prog, char* uniform, Vec4 vec);
extern "C" void ZR_SetProgM4x4(GLuint prog, char* uniform, f32* matrix);
extern "C" void ZR_PrepareTextureUnit1D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char* uniformName,
    GLint texture,
    GLint sampler);

extern "C" void ZR_PrepareTextureUnit2D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char* uniformName,
    GLint texture,
    GLint sampler);

extern "C" void ZR_PrepareTextureUnitCubeMap(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char* uniformName,
    GLint texture,
    GLint sampler);

extern "C" void ZRGL_SetupProg_Text(
    M4x4* projection,
    M4x4* modelView,
    i32 diffuseTexHandle,
    i32 charSheetStencilTexHandle,
    Colour fontColour,
    Colour bgColour);

extern "C" void ZR_DrawMeshGroupTest(
    Transform* camera,
    ZRDrawGroup* group,
    ZRDrawObj* objects,
    i32 numObjects,
    ScreenInfo* scrInfo,
    ZRGroupingStats* stats);

extern "C" void ZR_DrawMeshGroupFallback(
    Transform* camera,
    ZRDrawGroup* group,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats);

extern "C" void ZRGL_DrawDebugQuad(
    Vec2 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax,
    i32 texHandle, f32 aspectRatio, i32 bTransparent);

extern "C" void ZRGL_UploadTexture(u8* pixels, i32 width, i32 height, u32* handle);
extern "C" void ZRGL_UploadMesh(MeshData* data, ZRMeshHandles* result, u32 flags);

extern "C" f64 ZRGL_QueryClock();
extern "C" void ZRGL_ClearColourDefault();
extern "C" i32 ZRGL_GetLightMode();
extern "C" f32 ZRGL_GetInterpolate();

extern "C" ZRAssetDB* AssetDb();

///////////////////////////////////////////////////////////
// Clear loaded geometry
///////////////////////////////////////////////////////////
extern "C" void ZRGL_ClearBoundGeometry();

///////////////////////////////////////////////////////////
// Upload/Data for draw calls
///////////////////////////////////////////////////////////

extern "C" void ZRGL_UpdateStats(f64 swapMS, f64 frameMS);
extern "C" void ZR_UploadDataTexture();
extern "C" void ZR_UploadDBTex(ZRDBTexture* tex);
extern "C" void ZR_UploadDBMesh(ZRDBMesh* mesh);
extern "C" void ZRGL_SetupProjection(M4x4* target, i32 mode, f32 aspectRatio);
extern "C" ZRMeshDrawHandles ZRGL_ExtractDrawHandles(
    ZRAssetDB* db, i32 meshIndex, i32 materialIndex);

#endif // ZQF_GL_INTERNAL_H
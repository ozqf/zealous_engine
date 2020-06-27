#ifndef ZQF_GL_INTERNAL_H
#define ZQF_GL_INTERNAL_H
/**
 * Header for internal components of renderer
 */
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
#include "../ze_common/ze_common_full.h"
#include "../../lib/shaders.h"

// External interface
#include "../zqf_renderer.h"

// embedded assets
#include "../zr_embedded/zr_embedded.h"
// loaded assets
//#include "zr_asset_db.h"

#include "opengl_utils.h"
#include "../ze_common/ze_buf_block.h"

///////////////////////////////////////////////////////
// Frame buffers:
///////////////////////////////////////////////////////
struct ZRFrameBuffer
{
    GLuint fbo;
    GLuint colourTex;
    GLuint depthTex;
    GLuint depthRenderBuf;
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

static ZRGPUSpecs g_gpuLimits;

static i32 g_bDrawLocked = NO;
// TODO: Global flag for dropping debug data - pass as parameter instead?
static i32 g_verboseFrame = NO;

static GLuint g_cubemapHandle;

// Access to outside world
static ZRPlatform g_platform;

static ZRGBuffer g_gBuffer;

static f64 g_platformSwapMS = 0;
static f64 g_platformFrameMS = 0;

//static Vec4 g_clearColour;

static ZRMeshHandles g_cubeVAO;
static ZRMeshHandles g_inverseCubeVAO;
static ZRMeshHandles g_quadVAO;
static ZRMeshHandles g_spikeVAO;

static GLint g_defaultDiffuseHandle;

// For shadow map test
#define ZR_SHADOW_MAP_WIDTH 1024
#define ZR_SHADOW_MAP_HEIGHT 1024
static ZRFrameBuffer g_shadowMapFB;
static ZRFrameBuffer g_rendToTexFB;

static ZRShadowCaster g_shadow;

// Programs
static ZRShader g_programs[ZR_MAX_PROGRAMS];

// Samplers

// regular ones for geometry:
static GLuint g_samplerA;
static GLuint g_samplerB;
// These samplers should be configured to ignore mipmapping or any filtering.
// and to reliably fetch exact pixel values
static GLuint g_samplerDataTex2D;
static GLuint g_samplerDataTex1D;

/*
2D data texture of batch data sets
eg batches A,B,C,D,E:
height (eg 256)

|....................
|<D3><E0><E1>........
|<C2><C3><D0><D1><D2>
|<B0><B1><B2><C0><C1>
|<A0><A1><A2><A3><A4>
0-------width (eg 256)

index to pixel:
pixelX = index % imageWidth
pixelY = int(index / imageWidth);

*/
static ZRDataTexture g_dataTex2D;
static ZRDataTexture g_dataTex1D;

// Block of memory cleared every frame
#define ZQF_GL_SCRATCH_BYTES MegaBytes(1)
static ZEByteBuffer g_scratch = {};

// Note 1: A full RGBA32F texture at 1024x1024 is 16meg and really slow to upload!
//  512 however seems okay on my old GTX 970...so to do async texture uploads?
// Note 2: 1D textures are still limited to the same side length!
// so a 1D texture cannot be 1024 * 1024 wide!
#define ZQF_GL_DATA_TEXTURE_WIDTH 512

////////////////////////////////////////////////////////////
// internal functions
////////////////////////////////////////////////////////////
static ErrorCode ZRGL_CreateProgram(
    const char *vertexShader,
    const char *fragmentShader,
    char* shaderName,
    const i32 drawObjType,
    const i32 bIsBatchable,
    ZRShader* result);

static void ZR_SetProg1i(GLuint prog, char* uniform, GLint value);
static void ZR_SetProgVec4f(GLuint prog, char* uniform, Vec4 vec);
static void ZR_SetProgM4x4(GLuint prog, char* uniform, f32* matrix);
static void ZR_PrepareTextureUnit1D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char* uniformName,
    GLint texture,
    GLint sampler);

static void ZR_PrepareTextureUnit2D(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char* uniformName,
    GLint texture,
    GLint sampler);

static void ZR_PrepareTextureUnitCubeMap(
    GLint programId,
    GLint glTextureUnit,
    i32 textureUnit,
    char* uniformName,
    GLint texture,
    GLint sampler);

static void ZR_DrawMeshGroupTest(
    Transform* camera,
    ZRDrawGroup* group,
    ZRDrawObj* objects,
    i32 numObjects,
    ScreenInfo* scrInfo,
    ZRGroupingStats* stats);

static void ZR_DrawMeshGroupFallback(
    Transform* camera,
    ZRDrawGroup* group,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats);

static void ZRGL_DrawDebugQuad(
    Vec2 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax, i32 texHandle, f32 aspectRatio);

static void ZRGL_ClearColourDefault()
{
    glClearColor(0, 0, 0, 1);
}

static ZRAssetDB* AssetDb()
{
    return (ZRAssetDB*)g_platform.GetAssetDB();
}

///////////////////////////////////////////////////////////
// Upload/Data for draw calls
///////////////////////////////////////////////////////////
static void ZR_UploadDataTexture()
{
    ZRDataTexture* dataTex2D = &g_dataTex2D;
    
    Vec4* dataPixel = (Vec4*)dataTex2D->mem;
    // upload
    i32 w = dataTex2D->width;
	i32 h = dataTex2D->height;
    
	glBindTexture(GL_TEXTURE_2D, g_dataTex2D.handle);
	CHECK_GL_ERR
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, dataTex2D->mem);
    CHECK_GL_ERR
}

static void ZRGL_SetupProjection(M4x4* target, i32 mode, f32 aspectRatio)
{
    if (mode == ZR_PROJECTION_MODE_IDENTITY)
    {
        M4x4_SetToIdentity(target->cells);
    }
    else
    {
        COM_SetupDefault3DProjection(target->cells,
            aspectRatio);
    }
}

static ZRMeshDrawHandles ZRGL_ExtractDrawHandles(ZRAssetDB* db, i32 meshIndex, i32 materialIndex)
{
    ZRMeshDrawHandles h;

    ZRDBMesh* mesh = AssetDb()->GetMeshByIndex(AssetDb(), meshIndex);
	h.vao = mesh->handles.vao;
	h.vertCount = mesh->data.numVerts;

    ZRMaterial* mat = AssetDb()->GetMaterialByIndex(AssetDb(), materialIndex);
	h.diffuseHandle = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->diffuseTexIndex);
	h.emissiveHandle = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->emissionTexIndex);
	
    
    return h;
}

#endif // ZQF_GL_INTERNAL_H
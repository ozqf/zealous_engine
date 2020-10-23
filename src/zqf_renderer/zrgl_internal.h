#ifndef ZQF_GL_INTERNAL_H
#define ZQF_GL_INTERNAL_H
/**
 * Header for internal components of renderer
 */

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

#define ZRGL_MAX_ALLOCS 1024
static MallocItem g_mallocItems[ZRGL_MAX_ALLOCS];
static MallocList g_mallocs = 
{
    g_mallocItems, 0, ZRGL_MAX_ALLOCS, 0
};

static i32 g_debugFlags = 0;
static i32 g_lastFrameNumber = -1;
static i32 g_framesRendered = 0;
static f64 g_lastTimestamp;
static f32 g_interpolate = 0;

static ZRGPUSpecs g_gpuLimits;

static i32 g_bDrawLocked = NO;
// TODO: Global flag for dropping debug data - pass as parameter instead?
static i32 g_verboseFrame = NO;

static GLuint g_cubemapHandle;

// Access to outside world
static ZRPlatform g_platform;

static ZRGBuffer g_gBuffer;
static i32 g_lightingMode = 1;

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
static ZEBuffer g_scratch = {};

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
    Vec2 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax, i32 texHandle, f32 aspectRatio, i32 bTransparent);

static void ZRGL_UploadTexture(u8* pixels, i32 width, i32 height, u32* handle);
static void ZRGL_UploadMesh(MeshData* data, ZRMeshHandles* result, u32 flags);

static void ZRGL_ClearColourDefault()
{
    glClearColor(0, 0, 0, 1);
}

static ZRAssetDB* AssetDb()
{
    return g_platform.GetAssetDB();
}

///////////////////////////////////////////////////////////
// Clear loaded geometry
///////////////////////////////////////////////////////////
static void ZRGL_ClearBoundGeometry()
{
    glBindVertexArray(0);
    CHECK_GL_ERR
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERR
}

///////////////////////////////////////////////////////////
// Upload/Data for draw calls
///////////////////////////////////////////////////////////

extern "C" void ZRGL_UpdateStats(f64 swapMS, f64 frameMS)
{
    g_platformSwapMS = swapMS;
    g_platformFrameMS = frameMS;
}

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

static void ZR_UploadDBTex(ZRDBTexture* tex)
{
    u32 handle = 0;
    printf("Uploading tex %s\n", tex->header.fileName);
    ZRGL_UploadTexture((u8*)tex->data, tex->width, tex->height, &handle);
    tex->apiHandle = handle;
    tex->header.bIsUploaded = YES;
    //printf("Tex %s uploaded to handle %d\n", tex->header.fileName, tex->apiHandle);
}

static void ZR_UploadDBMesh(ZRDBMesh* mesh)
{
    ZRGL_UploadMesh(&mesh->data, &mesh->handles, 0);
    mesh->header.bIsUploaded = YES;
    //printf("Mesh %s uploaded to vao handle %d\n", mesh->header.fileName, mesh->handles.vao);
}

static void ZRGL_SetupProjection(M4x4* target, i32 mode, f32 aspectRatio)
{
    if (mode == ZR_PROJECTION_MODE_IDENTITY)
    {
        M4x4_SetToIdentity(target->cells);
    }
    else if (mode == ZR_PROJECTION_MODE_ORTHO_BASE)
    {
        COM_SetupOrthoProjection(target->cells, 1, aspectRatio);
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

    ZRDBMaterial* mat = AssetDb()->GetMaterialByIndex(AssetDb(), materialIndex);
	h.diffuseHandle = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->data.diffuseTexIndex);
	h.emissiveHandle = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->data.emissionTexIndex);
	
    
    return h;
}

#endif // ZQF_GL_INTERNAL_H
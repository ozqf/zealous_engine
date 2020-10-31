
#include "zrgl_internal.h"

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



extern "C" void ZRGL_ClearColourDefault()
{
    glClearColor(0, 0, 0, 1);
}

extern "C" f64 ZRGL_QueryClock() { return g_platform.QueryClock(); }
extern "C" i32 ZRGL_GetLightMode() { return g_lightingMode; }
extern "C" f32 ZRGL_GetInterpolate() { return g_interpolate; }

extern "C" ZRAssetDB* AssetDb()
{
    return g_platform.GetAssetDB();
}

///////////////////////////////////////////////////////////
// Clear loaded geometry
///////////////////////////////////////////////////////////
extern "C" void ZRGL_ClearBoundGeometry()
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

extern "C" void ZR_UploadDataTexture()
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

extern "C" void ZR_UploadDBTex(ZRDBTexture* tex)
{
    u32 handle = 0;
    printf("Uploading tex %s\n", tex->header.fileName);
    ZRGL_UploadTexture((u8*)tex->data, tex->width, tex->height, &handle);
    tex->apiHandle = handle;
    tex->header.bIsUploaded = YES;
    //printf("Tex %s uploaded to handle %d\n", tex->header.fileName, tex->apiHandle);
}

extern "C" void ZR_UploadDBMesh(ZRDBMesh* mesh)
{
    ZRGL_UploadMesh(&mesh->data, &mesh->handles, 0);
    mesh->header.bIsUploaded = YES;
    //printf("Mesh %s uploaded to vao handle %d\n", mesh->header.fileName, mesh->handles.vao);
}

extern "C" void ZRGL_SetupProjection(M4x4* target, i32 mode, f32 aspectRatio)
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

extern "C" ZRMeshDrawHandles ZRGL_ExtractDrawHandles(ZRAssetDB* db, i32 meshIndex, i32 materialIndex)
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


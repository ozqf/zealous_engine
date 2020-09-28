#ifndef ZRGL_INIT_H
#define ZRGL_INIT_H

#include "zrgl_internal.h"

internal void ZRGL_Fatal(const char* msg)
{
	g_platform.Error(msg);
}

static void ZRGL_PrintGPUInfo()
{
    const u8* vendor = glGetString(GL_VENDOR);
    const u8* renderer = glGetString(GL_RENDERER);
    i32 maxTexBufSize = -1;
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTexBufSize);
    printf("Vendor: %s, Renderer: %s\n", vendor, renderer);
    printf("Current memory: %dMB\n", g_gpuLimits.current_mem_kb / 1024);
    printf("Max texture size: %d\n", g_gpuLimits.maxTextureSize);
    printf("Max combined texture units: %d\n", g_gpuLimits.maxCombinedTextureUnits);
    printf("Maximum vertex attribs: %d\n", g_gpuLimits.maxVertexAttribs);
    printf("-- uniforms --\n");
    printf("Max Uniform block size: %dKB\n", g_gpuLimits.uniformBlocks.maxUniformBlockSize / 1024);
    printf("Uniform buffer offset alignment: %d\n",
        g_gpuLimits.uniformBlocks.uniformBufferOffsetAlignment);

    printf("-- Data texture --\n");
    printf("Batch data pixel count: %d\n", ZR_BATCH_DATA_STRIDE);
    printf("Max tex buf size: %d\n", maxTexBufSize);

    i32 drawBuffer;
    glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
    printf("Default draw buffer value: %d (0x%X)\n", drawBuffer, drawBuffer);
    CHECK_GL_ERR
}

static ZRGPUSpecs ZRGL_QueryGPUSpecs()
{
    ZRGPUSpecs specs = {};
    // this is an invalid enum on old intel integrated graphics.
    // not a biggy as we'll never intend to run properly on that mind.
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &specs.current_mem_kb);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &specs.maxTextureSize);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &specs.maxCombinedTextureUnits);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &specs.maxVertexAttribs);
    CHECK_GL_ERR

    // uniform stuff
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &specs.uniformBlocks.maxUniformBlockSize);
    CHECK_GL_ERR
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &specs.uniformBlocks.uniformBufferOffsetAlignment);
    CHECK_GL_ERR
    
    return specs;
}

extern "C" ErrorCode ZRGL_Init(i32 scrWidth, i32 scrHeight)
{
    printf("==== ZRGL - init ====\n");
    ZE_SetFatalError(ZRGL_Fatal);
    printf("Free alloc list\n");
    COM_FreeAll(&g_mallocs);

    // Scan GPU info
    g_gpuLimits = ZRGL_QueryGPUSpecs();
    ZRGL_PrintGPUInfo();

    // Global settings
    ZRGL_ClearColourDefault();
    glLineWidth(3);
    CHECK_GL_ERR

    // v-sync - off
    // want to cap frame rate manually... v-sync gives me jip.
    const i32 swapInterval = 0;
    printf("Swap interval - %d\n", swapInterval);
    glfwSwapInterval(swapInterval);

    // Depth buffer
	glEnable(GL_DEPTH_TEST);
    CHECK_GL_ERR
	glDepthFunc(GL_LESS);
    CHECK_GL_ERR
    glEnable(GL_CULL_FACE);
    CHECK_GL_ERR

    /////////////////////////////////////////
    // Default samplers
    /////////////////////////////////////////
    glGenSamplers(1, &g_samplerA);
    CHECK_GL_ERR
    glSamplerParameteri(g_samplerA, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECK_GL_ERR
    glSamplerParameteri(g_samplerA, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    CHECK_GL_ERR
    // Lightmap
    glGenSamplers(1, &g_samplerB);
    CHECK_GL_ERR
    glSamplerParameteri(g_samplerB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECK_GL_ERR
    glSamplerParameteri(g_samplerB, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    CHECK_GL_ERR

    /////////////////////////////////////////
    // Alloc scratch space
    /////////////////////////////////////////
    if (g_scratch.start == NULL)
    {
        g_scratch = Buf_FromMalloc(malloc(ZQF_GL_SCRATCH_BYTES), ZQF_GL_SCRATCH_BYTES);
    }

    /////////////////////////////////////////
    // Data textures
    /////////////////////////////////////////
    
    // Samplers for data textures
    // Make sure filtering and mip-mapping are disabled!
    glGenSamplers(1, &g_samplerDataTex2D);
    CHECK_GL_ERR
    glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    
    glGenSamplers(1, &g_samplerDataTex1D);
    CHECK_GL_ERR
	glSamplerParameteri(g_samplerDataTex1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glSamplerParameteri(g_samplerDataTex1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    
    // Create a 2D f32 RGBA texture
    g_dataTex2D = ZRGL_CreateDataTexture2D(&g_platform, ZQF_GL_DATA_TEXTURE_WIDTH);
    #if 1 // for debugging, print data capacity:
    i32 dataTex2DPixelCount = g_dataTex2D.width * g_dataTex2D.height;
    i32 dataTex2DCapacity = dataTex2DPixelCount / ZR_BATCH_DATA_STRIDE;
    i32 perRowCapacity = ZQF_GL_DATA_TEXTURE_WIDTH / ZR_BATCH_DATA_STRIDE;
    printf("Created 2D data texture. %d pixels, %dKB, %d capacity\n",
        dataTex2DPixelCount, g_dataTex2D.numBytes / 1024, dataTex2DCapacity);
    printf("  (%d per row)\n", perRowCapacity);
    #endif
    

	// Create a 1D f32 RGBA texture
	g_dataTex1D = ZRGL_CreateDataTexture1D(&g_platform, g_gpuLimits.maxTextureSize);
    printf("Created 1D data texture. %d pixels, %dKB\n",
        g_dataTex1D.width, g_dataTex1D.numBytes / 1024);
    
    /////////////////////////////////////////
    // Shadow maps/frame buffers
    /////////////////////////////////////////

    g_gBuffer = ZRGL_CreateGBuffer(scrWidth, scrHeight);
    
    g_shadowMapFB = ZRGL_CreateShadowMapBuffers(
        ZR_SHADOW_MAP_WIDTH,
        ZR_SHADOW_MAP_HEIGHT);

    g_rendToTexFB = ZRGL_CreateRenderToTextureBuffers(
        ZR_SHADOW_MAP_WIDTH,
        ZR_SHADOW_MAP_HEIGHT);
    
    /////////////////////////////////////////
    // Programs
    /////////////////////////////////////////
    
    ErrorCode err;

    err = ZRGL_CreateProgram(
        fallback_vert_text,
        fallback_frag_text,
        "Fallback",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_FALLBACK]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        single_vert_text,
        single_frag_text,
        "single",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_TEST]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        block_colour_vert_text,
        block_colour_frag_text,
        "block_colour",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_BLOCK_COLOUR]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        batch_vert_text,
        batch_frag_text,
        "Batch",
        ZR_DRAWOBJ_TYPE_MESH,
        YES,
        &g_programs[ZR_SHADER_TYPE_BATCHED]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        text_vert_text,
        text_frag_text,
        "Text",
        ZR_DRAWOBJ_TYPE_TEXT,
        NO,
        &g_programs[ZR_SHADER_TYPE_TEXT]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        skybox_vert_text,
        skybox_frag_text,
        "Skybox",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_SKYBOX]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        shadow_map_vert_text,
        shadow_map_frag_text,
        "ShadowMap",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_SHADOW_MAP]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        text_vert_text,
        debug_shadow_map_frag_text,
        "DebugShadowMap",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_SHADOW_MAP_DEBUG]);
    if (err != ZE_ERROR_NONE) { return err; }

    
    err = ZRGL_CreateProgram(
        gbuffer_create_vert_text,
        gbuffer_create_frag_text,
        "BuildGBuffer",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER]);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_combine_vert_text,
        gbuffer_combine_frag_text,
        "CombineGBuffer",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_COMBINE_GBUFFER]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        gbuffer_light_direct_vert_text,
        gbuffer_light_point_frag_text,
        "GBufferLightPoint",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_POINT]);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_light_direct_vert_text,
        gbuffer_light_direct_frag_text,
        "GBufferLightDirect",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_DIRECT]);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_light_volume_vert_text,
        gbuffer_light_volume_frag_text,
        "GBufferLightVolume",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_VOLUME]);
    if (err != ZE_ERROR_NONE) { return err; }

    /////////////////////////////////////////
    // Assets
    /////////////////////////////////////////

    // build test altas
    ZR_AtlasInit();

    #if 0
    // Setup assets
    ZRAssetUploader uploader = {};
    uploader.UploadTexture = ZRGL_UploadTexture;
    uploader.UploadMesh = ZRGL_UploadMesh;

    ZRDB_AttachUploader((ZRAssetDB*)g_platform.GetAssetDB(), uploader);
    #endif
    return ZE_ERROR_NONE;
}

extern "C" void ZRGL_UpdateStats(f64 swapMS, f64 frameMS)
{
    g_platformSwapMS = swapMS;
    g_platformFrameMS = frameMS;
}

static void ZR_UploadDBTex(ZRDBTexture* tex)
{
    u32 handle = 0;
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

/**
 * Check over the asset database and upload anything
 * that is not uploaded yet
 */
extern "C" void ZRGL_CheckForUploads(i32 bVerbose)
{
    if (bVerbose) { printf("=== ZRGL - Check for uploads ===\n"); }
    ZRAssetDB* db = AssetDb();
    i32 numTextures = db->GetNumTextures(db);
    if (bVerbose) { printf("--- %d textures total ---\n", numTextures); }
    for (i32 i = 0; i < numTextures; ++i)
    {
        ZRDBTexture* tex = db->GetTextureByIndex(db, i);
        if (tex->header.bIsUploaded == NO)
        {
            if (bVerbose) { printf("Tex %d: %s is not uploaded\n", i, tex->header.fileName); }
            ZR_UploadDBTex(tex);
        }
    }
    i32 numMeshes = db->GetNumMeshes(db);
    if (bVerbose) { printf("--- %d meshes total ---\n", numMeshes); }
    for (i32 i = 0; i < numMeshes; ++i)
    {
        ZRDBMesh* mesh = db->GetMeshByIndex(db, i);
        if (mesh->header.bIsUploaded == NO || mesh->header.bIsDirty == YES)
        {
            if (bVerbose) { printf("Mesh %d: (%d/%d verts) %s is not uploaded\n",
                i, mesh->data.numVerts, mesh->data.maxVerts, mesh->header.fileName); }
            ZR_UploadDBMesh(mesh);
        }
    }
    if (bVerbose) { printf("\n"); }
}

extern "C" void ZRGL_Link(ZRPlatform platform)
{
    printf("ZRGL - link\n");
    g_platform = platform;
}
#endif // ZRGL_INIT_H
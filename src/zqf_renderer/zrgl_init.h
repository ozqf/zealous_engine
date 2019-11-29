#ifndef ZRGL_INIT_H
#define ZRGL_INIT_H

#include "zrgl_internal.h"

static void ZRGL_PrintGPUInfo()
{
    const u8* vendor = glGetString(GL_VENDOR);
    const u8* renderer = glGetString(GL_RENDERER);
    printf("Vendor: %s, Renderer: %s\n", vendor, renderer);
    printf("Current memory: %dMB\n", g_gpuLimits.current_mem_kb / 1024);
    printf("Max texture size: %d\n", g_gpuLimits.maxTextureSize);
    printf("Max combined texture units: %d\n", g_gpuLimits.maxCombinedTextureUnits);
    printf("Maximum vertex attribs: %d\n", g_gpuLimits.maxVertexAttribs);
    printf("Batch data pixel count: %d\n", ZR_BATCH_DATA_STRIDE);

    i32 drawBuffer;
    glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
    printf("Default draw buffer value: %d (0x%X)\n", drawBuffer, drawBuffer);
}

static ErrorCode ZRGL_Impl_Init(i32 scrWidth, i32 scrHeight)
{
    printf("==== RENDERER - init ====\n");
    
    // Scan GPU info
    g_gpuLimits = {};
    // this is an invalid enum on old intel integrated graphics.
    // not a biggy as we'll never intend to run properly on that mind.
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &g_gpuLimits.current_mem_kb);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_gpuLimits.maxTextureSize);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &g_gpuLimits.maxCombinedTextureUnits);
    CHECK_GL_ERR
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &g_gpuLimits.maxVertexAttribs);
    CHECK_GL_ERR
    ZRGL_PrintGPUInfo();
    CHECK_GL_ERR
    
    // Global settings
    ZRGL_ClearColourDefault();
    glLineWidth(3);
    CHECK_GL_ERR

    // v-sync
    glfwSwapInterval(0);

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
    glSamplerParameteri(g_samplerB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(g_samplerB, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    /////////////////////////////////////////
    // Alloc scratch space
    /////////////////////////////////////////
    g_scratch = Buf_FromMalloc(malloc(ZQF_GL_SCRATCH_BYTES), ZQF_GL_SCRATCH_BYTES);

    /////////////////////////////////////////
    // Data textures
    /////////////////////////////////////////
    
    // Samplers for data textures
    // Make sure filtering and mip-mapping are disabled!
    glGenSamplers(1, &g_samplerDataTex2D);
    glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glGenSamplers(1, &g_samplerDataTex1D);
	glSamplerParameteri(g_samplerDataTex1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(g_samplerDataTex1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
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
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_FALLBACK]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        single_vert_text,
        single_frag_text,
        "single",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_TEST]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        block_colour_vert_text,
        block_colour_frag_text,
        "block_colour",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_BLOCK_COLOUR]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        batch_vert_text,
        batch_frag_text,
        "Batch",
        ZR_DRAWOBJ_TYPE_PREFAB,
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
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_SKYBOX]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        shadow_map_vert_text,
        shadow_map_frag_text,
        "ShadowMap",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_SHADOW_MAP]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        text_vert_text,
        debug_shadow_map_frag_text,
        "DebugShadowMap",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_SHADOW_MAP_DEBUG]);
    if (err != ZE_ERROR_NONE) { return err; }

    
    err = ZRGL_CreateProgram(
        gbuffer_create_vert_text,
        gbuffer_create_frag_text,
        "BuildGBuffer",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER]);
    
    err = ZRGL_CreateProgram(
        gbuffer_combine_vert_text,
        gbuffer_combine_frag_text,
        "CombineGBuffer",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_COMBINE_GBUFFER]);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        gbuffer_light_direct_vert_text,
        gbuffer_light_point_frag_text,
        "GBufferLightPoint",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_POINT]);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_light_direct_vert_text,
        gbuffer_light_direct_frag_text,
        "GBufferLightDirect",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_DIRECT]);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_light_volume_vert_text,
        gbuffer_light_volume_frag_text,
        "GBufferLightVolume",
        ZR_DRAWOBJ_TYPE_PREFAB,
        NO,
        &g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_VOLUME]);
    if (err != ZE_ERROR_NONE) { return err; }

    /////////////////////////////////////////
    // Assets
    /////////////////////////////////////////

    // Setup export
    g_assets.GetMaterialIndexByName = ZRDB_GetTexIndexByName;
    g_assets.GetMeshIndexByName = ZRDB_GetMeshIndexByName;
    g_assets.GetTexIndexByName = ZRDB_GetTexIndexByName;

    ZE_SET_ZERO(g_prefabs, sizeof(ZRPrefab) * ZR_MAX_PREFABS);
    
    ZRGL_LoadDefaultPrefabs(NO);

    return ZE_ERROR_NONE;
}

// TODO: Bleh just make this OO
extern "C" ZRAssetDB* ZRImpl_GetAssetDB()
{
    return &g_assets;
}

extern "C" ZRRenderer ZR_Link(ZRPlatform platform)
{
    printf("==== Renderer Link ====\n");
    g_platform = platform;
    ZRRenderer r = {};
    r.Init = ZRGL_Impl_Init;
    r.DrawFrameForward = ZRImpl_DrawFrameForward;
    r.DrawFrameDeferred = ZRImpl_DrawFrameDeferred;
    r.GetAssetDB = ZRImpl_GetAssetDB;
    r.isValid = YES;
    return r;
}
#endif // ZRGL_INIT_H
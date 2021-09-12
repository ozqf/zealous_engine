#include "../ze_opengl_internal.h"

struct ZRGLSpriteBatchItem
{
    Vec4 posAndRadians;
    Vec4 uvs;
    Vec4 colour;
    Vec4 scaleAndOffset;
};

ze_external void ZRSandbox_DrawSpriteBatch()
{
    //////////////////////////////////////////////////
    // constants
    //////////////////////////////////////////////////
    const i32 dataTextureSize = 128;
    const i32 totalDataPixels = dataTextureSize * dataTextureSize;

    //////////////////////////////////////////////////
    // persist vars
    //////////////////////////////////////////////////
    local_persist i32 g_bInitialised = NO;
    local_persist ZRShader g_shader = {};
    local_persist i32 g_meshId;
    local_persist i32 g_diffuseTexHandle;

    // SSBO
    local_persist GLuint ssbo;

    // data texture
    local_persist Vec4 g_dataPixels[totalDataPixels];
    local_persist GLuint g_dataTextureHandle;
    local_persist GLuint g_samplerDataTex2D;

    //////////////////////////////////////////////////
    // local vars
    //////////////////////////////////////////////////
    zErrorCode err;

    //////////////////////////////////////////////////
    // init
    //////////////////////////////////////////////////
    if (g_bInitialised == NO)
    {
        g_bInitialised = YES;
        err = ZRGL_CreateProgram(
            draw_sprite_batch_vert_text,
            draw_sprite_batch_frag_text,
            "sprite_batch_test",
            ZR_DRAWOBJ_TYPE_QUAD,
            YES,
            &g_shader);
        ZE_ASSERT(err == ZE_ERROR_NONE, "create prog failed")

        g_meshId = ZAssets_GetMeshByName(ZE_EMBEDDED_QUAD_NAME)->header.id;

        // create a little sprite sheet
        ZRTexture* tex = ZAssets_AllocTex(32, 32, "sprite_batch_test_atlas");
        ZGen_FillTexture(tex, COLOUR_U32_BLACK);
        ZGen_FillTextureRect(tex, COLOUR_U32_WHITE, { 16, 16 }, { 16, 16 });
        g_diffuseTexHandle = ZRGL_GetTextureHandle(tex->header.id);
        
        // allocate data texture and sampler for it
        glGenTextures(1, &g_dataTextureHandle);
        CHECK_GL_ERR
        glBindTexture(GL_TEXTURE_2D, g_dataTextureHandle);
        CHECK_GL_ERR


        #if 1 // try and set just a portion of the data texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
            dataTextureSize, dataTextureSize, 0, GL_RGBA, GL_FLOAT, NULL);
        CHECK_GL_ERR
        i32 i = 0;
        g_dataPixels[i++] = { 1, 0, 0, 1 };
        g_dataPixels[i++] = { 0, 1, 0, 1 };
        g_dataPixels[i++] = { 0, 0, 1, 1 };
        g_dataPixels[i++] = { 1, 0, 1, 1 };

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 1,  GL_RGBA, GL_FLOAT, g_dataPixels);
        CHECK_GL_ERR
        #endif

        #if 0 // set the whole data texture
        for (i32 i = 0; i < totalDataPixels; ++i)
        {
            g_dataPixels[i] = COLOUR_F32_WHITE;
        }
        
        // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dataTextureSize, dataTextureSize,  GL_RGBA, GL_FLOAT, g_dataPixels);
        printf("Uploading %d by %d data pixels\n", dataTextureSize, dataTextureSize);
        glBindTexture(GL_TEXTURE_2D, g_dataTextureHandle);
        CHECK_GL_ERR
        //Platform_DebugBreak();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
            dataTextureSize, dataTextureSize, 0, GL_RGBA, GL_FLOAT, g_dataPixels);
        CHECK_GL_ERR
        // glBindTexture(GL_TEXTURE_2D, g_dataTextureHandle);
        // CHECK_GL_ERR
        // glTexSubImage2D(
        //     GL_TEXTURE_2D, 0, 0, 0,
        //     dataTextureSize, dataTextureSize, GL_RGBA, GL_FLOAT, g_dataPixels);
        // CHECK_GL_ERR
        #endif

        // ...okay lets try an SSBO instead...?
        #if 0
        glGenBuffers(1, &ssbo);
        CHECK_GL_ERR
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        CHECK_GL_ERR
        #endif


        // Samplers for data textures
        // Make sure filtering and mip-mapping are disabled!
        glGenSamplers(1, &g_samplerDataTex2D);
        CHECK_GL_ERR
        glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        CHECK_GL_ERR
        glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        CHECK_GL_ERR
    }
    #if 1
    //////////////////////////////////////////////////
    // execute
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    // draw

    // acquire mesh handles
    ZRMeshHandles* mesh = ZRGL_GetMeshHandles(g_meshId);
    
    // clear
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // not interested in depth buffer for this
    glClearColor(0.5f, 0, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    i32 instanceCount = 4;

    // bind gpu objects
    glBindVertexArray(mesh->vao);
    CHECK_GL_ERR
    glUseProgram(g_shader.handle);
    CHECK_GL_ERR

    M4x4_CREATE(projection)
    M4x4_CREATE(model)

    // ZR_SetProg1i(g_shader.handle, "u_instanceCount", instanceCount);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", projection.cells);
    
    ZR_PrepareTextureUnit2D(
        g_shader.handle, GL_TEXTURE0, 0, "u_diffuseTex", g_diffuseTexHandle, 0);
    ZR_PrepareTextureUnit2D(
        g_shader.handle, GL_TEXTURE2, 2, "u_dataTexture", g_dataTextureHandle, g_samplerDataTex2D
    );

    // draw
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexCount, instanceCount);
    CHECK_GL_ERR
    #endif
}

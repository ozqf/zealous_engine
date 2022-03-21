#include "../ze_opengl_internal.h"

// for rand()
#include <stdlib.h>

#ifndef RANDF
#define RANDF ((f32)rand() / RAND_MAX)
#define RANDF_RANGE(minValueF, maxValueF) (RANDF * (maxValueF - minValueF) + minValueF)
#endif

/*struct ZRGLSpriteBatchItem
{
    Vec4 posAndRadians;
    Vec4 uvs;
    Vec4 scaleAndOffset;
    Vec4 colour;
};*/

ze_external void ZRSandbox_DrawSpriteBatch()
{
    //////////////////////////////////////////////////
    // constants
    //////////////////////////////////////////////////
    const i32 dataTextureSize = 256; //512;
    const i32 totalDataPixels = dataTextureSize * dataTextureSize;
    const f32 range = 3.f;
    

    //////////////////////////////////////////////////
    // persist vars
    //////////////////////////////////////////////////
    local_persist i32 g_bInitialised = NO;
    local_persist ZRShader g_shader = {};
    local_persist i32 g_meshId;
    local_persist i32 g_diffuseTexHandle;
    local_persist i32 g_instanceCount;

    local_persist Transform camera;
    local_persist f64 g_lastTickTime;

    // SSBO
    local_persist GLuint ssbo;

    // data texture
    local_persist Vec4 g_dataPixels[totalDataPixels];
    local_persist GLuint g_dataTextureHandle;
    local_persist GLuint g_samplerDataTex2D;
    local_persist i32 g_dataPixelStride = 2;

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
        g_lastTickTime = Platform_QueryClock();
        Transform_SetToIdentity(&camera);

        err = ZRGL_CreateProgram(
            draw_sprite_batch_vert_text,
            draw_sprite_batch_frag_text,
            "sprite_batch_test",
            &g_shader);
        ZE_ASSERT(err == ZE_ERROR_NONE, "create prog failed")

        g_meshId = ZAssets_GetMeshByName(ZE_EMBEDDED_QUAD_NAME)->header.id;

        // create a little sprite sheet
        ZRTexture* tex = ZAssets_AllocTex(32, 32, "sprite_batch_test_atlas");
        ZGen_FillTextureRect(tex, COLOUR_U32_RED, { 0, 0 }, { 16, 16 });
        ZGen_FillTextureRect(tex, COLOUR_U32_GREEN, { 16, 0 }, { 16, 16 });
        ZGen_FillTextureRect(tex, COLOUR_U32_BLUE, { 0, 16 }, { 16, 16 });
        ZGen_FillTextureRect(tex, COLOUR_U32_YELLOW, { 16, 16 }, { 16, 16 });
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

        g_dataPixelStride = 2;
        i32 i = 0;
        i32 batchMax = totalDataPixels / g_dataPixelStride;
        // i32 batchMax = 1;
        printf("Total data pixels: %d, stride %d, batch max %d\n",
            totalDataPixels, g_dataPixelStride, batchMax);
        
        #if 0 // hard coded
        g_dataPixels[i++] = { -0.5f, -0.5f, 0, 1 }; // pos and rot
        g_dataPixels[i++] = { 0.25f, 0.25f, 0.25f, 0.25f }; // uvs min/max
        g_dataPixelStride = 2;

        g_dataPixels[i++] = { 0.5f, -0.5f, 0, 1 };
        g_dataPixels[i++] = { 0.75f, 0.25f, 0.75f, 0.25f };

        g_dataPixels[i++] = { 0.5f, 0.5f, 0, 1 };
        g_dataPixels[i++] = { 0.75f, 0.75f, 0.75f, 0.75f };

        g_dataPixels[i++] = { -0.5f, 0.5f, 0, 1 };
        g_dataPixels[i++] = { 0.25f, 0.75f, 0.25f, 0.75 };

        g_dataPixels[i++] = { 0, 0, 0, 1 };
        g_dataPixels[i++] = { 0, 0, 1, 1 };
        #endif
        
        #if 1
        for (i = 0; i < batchMax; i += 1)
        {
            i32 pixel = g_dataPixelStride * i;
            g_dataPixels[pixel] =
            {
                RANDF_RANGE(-range, range),
                RANDF_RANGE(-range, range),
                RANDF_RANGE(-4, -16),
                1
            };
            g_dataPixels[pixel + 1] = { 0.75f, 0.25f, 0.75f, 0.25f };
        }

        #endif

        g_instanceCount = i;
        printf("Drawing %d instances of %d max\n", g_instanceCount, batchMax);
        printf("\tData texture is %lldKB\n", (totalDataPixels * sizeof(Vec4)) / 1024);

        // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, i, 1,  GL_RGBA, GL_FLOAT, g_dataPixels);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dataTextureSize, dataTextureSize,  GL_RGBA, GL_FLOAT, g_dataPixels);
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

    // calculate delta
    f64 now = Platform_QueryClock();
    f64 delta = now - g_lastTickTime;

    #if 1 // update data texture to check data refreshing
    const i32 numColours = 4;
    const Vec4 colours[numColours] =
    {
        { 0.25f, 0.25f, 0.25f, 0.25f },
        { 0.75f, 0.25f, 0.75f, 0.25f },
        { 0.75f, 0.75f, 0.75f, 0.75f },
        { 0.25f, 0.75f, 0.25f, 0.75f }
    };

    for (i32 i = 0; i < g_instanceCount; i += 1)
    {
        i32 pixel = g_dataPixelStride * i;
        g_dataPixels[pixel] =
        {
            RANDF_RANGE(-range, range),
            RANDF_RANGE(-range, range),
            RANDF_RANGE(-4, -16),
            1
        };
        g_dataPixels[pixel + 1] = colours[i % numColours];
    }
    glBindTexture(GL_TEXTURE_2D, g_dataTextureHandle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dataTextureSize, dataTextureSize,  GL_RGBA, GL_FLOAT, g_dataPixels);
        CHECK_GL_ERR
    #endif

    //////////////////////////////////////////////////
    // draw

    // acquire mesh handles
    ZRMeshHandles* mesh = ZRGL_GetMeshHandles(g_meshId);
    
    // clear
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind gpu objects
    glBindVertexArray(mesh->vao);
    CHECK_GL_ERR
    glUseProgram(g_shader.handle);
    CHECK_GL_ERR
    
    // ZR_SetProg1i(g_shader.handle, "u_instanceCount", g_instanceCount);
    ZR_SetProg1i(g_shader.handle, "u_dataStride", g_dataPixelStride);
    ZR_SetProg1i(g_shader.handle, "u_dataTexSize", dataTextureSize);

    M4x4_CREATE(projection)
    ZE_SetupDefault3DProjection(projection.cells, Window_GetInfo().aspect);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", projection.cells);

    M4x4_CREATE(view)
    Transform_ToM4x4(&camera, &view);
    ZR_SetProgM4x4(g_shader.handle, "u_view", view.cells);
    
    ZR_PrepareTextureUnit2D(
        g_shader.handle, GL_TEXTURE0, 0, "u_diffuseTex", g_diffuseTexHandle, 0);
    ZR_PrepareTextureUnit2D(
        g_shader.handle, GL_TEXTURE2, 2, "u_dataTexture", g_dataTextureHandle, g_samplerDataTex2D
    );

    // draw
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexCount, g_instanceCount);
    CHECK_GL_ERR
    #endif
}

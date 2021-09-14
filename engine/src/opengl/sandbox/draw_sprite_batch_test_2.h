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

ze_external void ZRSandbox_DrawSpriteBatch_2()
{
    //////////////////////////////////////////////////
    // constants
    //////////////////////////////////////////////////
    const i32 dataTextureSize = 512;
    const i32 totalDataPixels = dataTextureSize * dataTextureSize;
    const f32 rangeX = 25.f;
    const f32 rangeY = 25.f;
    const f32 rangeZ = 25.f;

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
    local_persist f32 g_yawDegrees = 0;

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
            draw_sprite_batch_2_vert_text,
            draw_sprite_batch_frag_text,
            "sprite_batch_test_2",
            ZR_DRAWOBJ_TYPE_QUAD,
            YES,
            &g_shader);
        ZE_ASSERT(err == ZE_ERROR_NONE, "create prog failed")

        g_meshId = ZAssets_GetMeshByName(ZE_EMBEDDED_QUAD_NAME)->header.id;

        GetEngine().input.AddAction(Z_INPUT_CODE_LEFT, 0, "turn_left");
        GetEngine().input.AddAction(Z_INPUT_CODE_RIGHT, 0, "turn_right");

        GetEngine().input.AddAction(Z_INPUT_CODE_W, 0, "move_forward");
        GetEngine().input.AddAction(Z_INPUT_CODE_S, 0, "move_backward");
        GetEngine().input.AddAction(Z_INPUT_CODE_A, 0, "move_left");
        GetEngine().input.AddAction(Z_INPUT_CODE_D, 0, "move_right");

        GetEngine().input.AddAction(Z_INPUT_CODE_R, 0, "reset");

        // create a little sprite sheet
        ZRTexture *tex = ZAssets_AllocTex(32, 32, "sprite_batch_test_atlas_2");
        ZGen_FillTextureRect(tex, COLOUR_U32_RED, {0, 0}, {16, 16});
        ZGen_FillTextureRect(tex, COLOUR_U32_GREEN, {16, 0}, {16, 16});
        ZGen_FillTextureRect(tex, COLOUR_U32_BLUE, {0, 16}, {16, 16});
        ZGen_FillTextureRect(tex, COLOUR_U32_YELLOW, {16, 16}, {16, 16});
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

        Vec4 uvs[4] = {
            {0.25f, 0.25f, 0.25f, 0.25f},
            {0.75f, 0.25f, 0.75f, 0.25f},
            {0.75f, 0.75f, 0.75f, 0.75f},
            {0.25f, 0.75f, 0.25f, 0.75}};

#if 0 // hard coded
        batchMax = 5;
        g_dataPixels[i++] = { 0, 0, -4, 1 }; // pos and rot
        g_dataPixels[i++] = { 0.25f, 0.25f, 0.25f, 0.25f }; // uvs min/max
        g_dataPixelStride = 2;

        g_dataPixels[i++] = { -2, 0, -4, 1 };
        g_dataPixels[i++] = { 0.75f, 0.25f, 0.75f, 0.25f };

        g_dataPixels[i++] = { 2, 0, -4, 1 };
        g_dataPixels[i++] = { 0.75f, 0.75f, 0.75f, 0.75f };

        g_dataPixels[i++] = { 0, 2, -4, 1 };
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
                    RANDF_RANGE(-rangeX, rangeX),
                    RANDF_RANGE(-rangeY, rangeY),
                    RANDF_RANGE(-rangeZ, rangeZ),
                    1};
            // g_dataPixels[pixel + 1] = { 0.75f, 0.25f, 0.75f, 0.25f };
            g_dataPixels[pixel + 1] = uvs[i % 4];
        }

#endif

        g_instanceCount = i;
        printf("Drawing %d instances of %d max\n", g_instanceCount, batchMax);
        printf("\tData texture is %lldKB\n", (totalDataPixels * sizeof(Vec4)) / 1024);

        // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, i, 1,  GL_RGBA, GL_FLOAT, g_dataPixels);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dataTextureSize, dataTextureSize, GL_RGBA, GL_FLOAT, g_dataPixels);
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
    f32 delta = (f32)(now - g_lastTickTime);
    g_lastTickTime = now;

#if 0 // update data texture to check data refreshing
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
        g_dataPixels[pixel + 1] = { 0.75f, 0.25f, 0.75f, 0.25f };
    }
    glBindTexture(GL_TEXTURE_2D, g_dataTextureHandle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dataTextureSize, dataTextureSize,  GL_RGBA, GL_FLOAT, g_dataPixels);
        CHECK_GL_ERR
#endif

    //////////////////////////////////////////////////
    // draw

    // acquire mesh handles
    ZRMeshHandles *mesh = ZRGL_GetMeshHandles(g_meshId);

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
    ZR_SetProg1i(g_shader.handle, "u_isBillboard", 1);

    M4x4_CREATE(projection)
        ZE_SetupDefault3DProjection(projection.cells, 16.f / 9.f);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", projection.cells);

    // rotate camera
    f32 cosVal = cosf((f32)g_lastTickTime / 2.f);
    f32 sinVal = sinf((f32)g_lastTickTime / 2.f);
    f32 x = (cosVal * 100);
    f32 z = (sinVal * 100);
    // f32 x = 0;
    // f32 z = -2;
    // f32 x = cosf(0) * 10;
    // f32 z = sinf(0) * 10;
    // 0, 1, -2 is visible
    // camera.pos.x = 0; //x;
    // camera.pos.y = 0;
    // camera.pos.z = -4;//0; //z;

#if 0 // rotate camera
    Vec3 target = { 0, 0, 0 };
    Vec3 toTarget =
    {
        target.x + camera.pos.x,
        target.y + camera.pos.y,
        target.z + camera.pos.z
    };
    Vec3_Normalise(&toTarget);
    Vec3 rot = Vec3_EulerAngles(toTarget);
    printf("Euler %.3f, %.3f, %.3f\n", rot.x, rot.y, rot.z);
    Transform_SetRotation(&camera, rot.x, rot.y, rot.z);
#endif
    #if 1
    ZEngine engine = GetEngine();
    if (engine.input.GetActionValue("reset"))
    {
        Transform_SetToIdentity(&camera);
        g_yawDegrees = 0;
    }

    if (engine.input.GetActionValue("turn_left"))
    {
        g_yawDegrees += 45.f * delta;
    }
    if (engine.input.GetActionValue("turn_right"))
    {
        g_yawDegrees -= 45.f * delta;
    }
    
    // Transform_SetRotation(&camera, 0, (45.f * sinVal) * DEG2RAD, 0);
    M3x3_SetToIdentity(camera.rotation.cells);
    // M3x3_RotateY(camera.rotation.cells, g_yawDegrees * DEG2RAD);
    Transform_SetRotation(&camera, 0, g_yawDegrees * DEG2RAD, 0);
    
    Vec3 move = {};
    if (engine.input.GetActionValue("move_forward")) { move.z -= 1; }
    if (engine.input.GetActionValue("move_backward")) { move.z += 1; }
    if (engine.input.GetActionValue("move_left")) { move.x -= 1; }
    if (engine.input.GetActionValue("move_right")) { move.x += 1; }
    Vec3 moveDir = M3x3_Calculate3DMove(&camera.rotation, move);
    Vec3_MulF(&moveDir, 10.f * delta);
    Vec3_AddTo(&camera.pos, moveDir);

    #endif

    M4x4_CREATE(view)
    Transform_ToViewMatrix(&camera, &view);
    ZR_SetProgM4x4(g_shader.handle, "u_view", view.cells);

    ZR_PrepareTextureUnit2D(
        g_shader.handle, GL_TEXTURE0, 0, "u_diffuseTex", g_diffuseTexHandle, 0);
    ZR_PrepareTextureUnit2D(
        g_shader.handle, GL_TEXTURE2, 2, "u_dataTexture", g_dataTextureHandle, g_samplerDataTex2D);

    // draw
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexCount, g_instanceCount);
    CHECK_GL_ERR
#endif
}

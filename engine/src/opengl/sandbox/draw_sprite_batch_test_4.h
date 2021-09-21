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

ze_external void ZRSandbox_DrawSpriteBatch_4()
{
    //////////////////////////////////////////////////
    // constants
    //////////////////////////////////////////////////
    const i32 dataTextureSize = 512;
    const i32 totalDataPixels = dataTextureSize * dataTextureSize;
    const f32 rangeX = 100.f;
    const f32 rangeY = 10.f;
    const f32 rangeZ = 100.f;

    //////////////////////////////////////////////////
    // persist vars
    //////////////////////////////////////////////////
    local_persist i32 g_bInitialised = NO;
    local_persist ZRShader g_batchDrawShader = {};
    local_persist i32 g_meshId;
    local_persist i32 g_diffuseTexHandle;
    local_persist i32 g_instanceCount;

    local_persist ZRShader g_blendShader = {};

    local_persist Transform camera;
    local_persist Transform cameraOrigin;
    local_persist f32 g_orthographicSize = 8;
    local_persist f32 g_originalOrthographicSize = 8;
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
        camera.pos.z = 1;
        cameraOrigin = camera;

        err = ZRGL_CreateProgram(
            draw_sprite_batch_3_vert_text,
            draw_sprite_batch_frag_text,
            "sprite_batch_test_3",
            &g_batchDrawShader);
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
        ZRTexture *tex = ZAssets_AllocTex(32, 32, "sprite_batch_test_atlas_3");
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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
                     dataTextureSize, dataTextureSize, 0, GL_RGBA, GL_FLOAT, NULL);
        CHECK_GL_ERR

        // pixel 0 is position, 1 is UVs, 2 is scale
        g_dataPixelStride = 3;
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

        #endif
        
        #if 1 // random scatter
        for (i = 0; i < batchMax; i += 1)
        {
            i32 pixel = g_dataPixelStride * i;
            g_dataPixels[pixel] =
                {
                    RANDF_RANGE(-rangeX, rangeX),
                    RANDF_RANGE(-rangeY, rangeY),
                    0,//RANDF_RANGE(-rangeZ, rangeZ),
                    RANDF_RANGE(0, 360) * DEG2RAD
                };
            g_dataPixels[pixel + 1] = uvs[i % 4];
            g_dataPixels[pixel + 2] = { RANDF_RANGE(0.1f, 1), RANDF_RANGE(0.1f, 1) };
        }
        #endif

        g_instanceCount = i;
        printf("Drawing %d instances of %d max\n", g_instanceCount, batchMax);
        printf("\tData texture is %lldKB\n", (totalDataPixels * sizeof(Vec4)) / 1024);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dataTextureSize, dataTextureSize, GL_RGBA, GL_FLOAT, g_dataPixels);
        CHECK_GL_ERR

        // Samplers for data textures
        // Make sure filtering and mip-mapping are disabled!
        glGenSamplers(1, &g_samplerDataTex2D);
        CHECK_GL_ERR
        glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        CHECK_GL_ERR
        glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        CHECK_GL_ERR
        
        /*
        Set up for drawing debug quad
        */
        err = ZRGL_CreateProgram(
            sandbox_draw_single_mesh_vert_text,
            sandbox_draw_single_mesh_frag_text,
            "draw_tiles_debug",
            &g_blendShader);
        g_blendShader.flags = ZR_SHADER_FLAG_BLEND | ZR_SHADER_FLAG_NO_DEPTH;
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
    glClearColor(0, 0, 0, 1);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind gpu objects
    glBindVertexArray(mesh->vao);
    CHECK_GL_ERR
    ZR_PrepareShader(&g_batchDrawShader);
    
    // ZR_SetProg1i(g_batchDrawShader.handle, "u_instanceCount", g_instanceCount);
    ZR_SetProg1i(g_batchDrawShader.handle, "u_dataStride", g_dataPixelStride);
    ZR_SetProg1i(g_batchDrawShader.handle, "u_dataTexSize", dataTextureSize);
    ZR_SetProg1i(g_batchDrawShader.handle, "u_isBillboard", 1);

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
    
    ZEngine engine = GetEngine();
    if (engine.input.GetActionValue("reset"))
    {
        // Transform_SetToIdentity(&camera);
        camera = cameraOrigin;
        g_yawDegrees = 0;
        g_orthographicSize = g_originalOrthographicSize;
    }

    /* No turning in orthographic!
    if (engine.input.GetActionValue("turn_left"))
    {
        g_yawDegrees += 90.f * delta;
    }
    if (engine.input.GetActionValue("turn_right"))
    {
        g_yawDegrees -= 90.f * delta;
    }
    */
    // Transform_SetRotation(&camera, 0, (45.f * sinVal) * DEG2RAD, 0);
    M3x3_SetToIdentity(camera.rotation.cells);
    // M3x3_RotateY(camera.rotation.cells, g_yawDegrees * DEG2RAD);
    Transform_SetRotation(&camera, 0, g_yawDegrees * DEG2RAD, 0);
    
    Vec3 move = {};
    // if (engine.input.GetActionValue("move_forward")) { move.z -= 1; }
    // if (engine.input.GetActionValue("move_backward")) { move.z += 1; }
    if (engine.input.GetActionValue("move_left")) { move.x -= 1; }
    if (engine.input.GetActionValue("move_right")) { move.x += 1; }
    Vec3 moveDir = M3x3_Calculate3DMove(&camera.rotation, move);
    Vec3_MulF(&moveDir, 10.f * delta);
    Vec3_AddTo(&camera.pos, moveDir);

    if (engine.input.GetActionValue("move_forward")) { g_orthographicSize -= 1.f * delta; }
    if (engine.input.GetActionValue("move_backward")) { g_orthographicSize += 1.f * delta; }

    M4x4_CREATE(projection)
    ZE_SetupDefault3DProjection(projection.cells, 16.f / 9.f);
    ZE_SetupOrthoProjection(projection.cells, g_orthographicSize, 16.f / 9.f);
    ZR_SetProgM4x4(g_batchDrawShader.handle, "u_projection", projection.cells);

    M4x4_CREATE(view)
    Transform_ToViewMatrix(&camera, &view);
    ZR_SetProgM4x4(g_batchDrawShader.handle, "u_view", view.cells);

    ZR_PrepareTextureUnit2D(
        g_batchDrawShader.handle, GL_TEXTURE0, 0, "u_diffuseTex", g_diffuseTexHandle, 0);
    ZR_PrepareTextureUnit2D(
        g_batchDrawShader.handle, GL_TEXTURE2, 2, "u_dataTexture", g_dataTextureHandle, g_samplerDataTex2D);

    // draw
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexCount, g_instanceCount);
    CHECK_GL_ERR

    #if 1 // draw overlay
    // draw tiles overlay
    ZR_PrepareShader(&g_blendShader);

    // bind gpu objects
    i32 screenQuadAssetId = ZAssets_GetMeshByName(ZE_EMBEDDED_SCREEN_QUAD_NAME)->header.id;
    i32 overlayVao = ZRGL_GetMeshHandles(screenQuadAssetId)->vao;
    glBindVertexArray(overlayVao);
    CHECK_GL_ERR
    // M4x4_CREATE(model)
    // M4x4_CREATE(modelView)
    // M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
    // M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
    M4x4_CREATE(screenspaceMV)
    // M4x4_BuildScale(screenspaceMV.cells, 2, 2, 2);
    M4x4_CREATE(screenspacePrj)
    ZR_SetProgM4x4(g_blendShader.handle, "u_modelView", screenspaceMV.cells);
    ZR_SetProgM4x4(g_blendShader.handle, "u_projection", screenspacePrj.cells);
    // Vec4 colour = { 0.5f, 0.5f, 0.5f, 0.5f };
    Vec4 colour = { 1, 0.5f, 0.5f, 0.5f };
    // ZR_SetProgVec4f(g_blendShader.handle, "u_colour", colour);
    glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
    
    #endif // draw overlay
#endif
}

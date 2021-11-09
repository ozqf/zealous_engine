/*
Sprite batching + Tile based shader lighting data.

Tiles
Linear 2d array, each cell being a index into a list of lights

Two data blobs:
> List of lights affecting the view in a linear array. type: Vec4
> 2D grid, with one axis being the tile index, and the other being the list of indices
    type: u16. each tile is a row, value in column 0 is the number of indices.

Lights stored in a Vec4 texture, MaxLightCount by DataPixelsPerLight
Vec4 x/y/z/radius
Vec4 r/g/b/strength

0-------DataPixelsPerLight
|<light 0 pixel 0>, <light 0 pixel 1>
|<light 1 pixel 0>, <light 1 pixel 1>
|<light 2 pixel 0>, <light 2 pixel 1>
|...etc
Max Light Count

16x16 == 256 screen tiles, u16 type
Column 0 is count of lights, rest are indices to lights table
0-------width
|cell 0 count, 0 to 254 indices...
|cell 1 - 0 to 256 indices...
|cell 2 ...etc
|...
|cell 255
height

Lookup example - pseudo code.
eg frag_pos of -0.5, -0.5 -> what lights affect this pixel?
    // convert frag pos to 0...1 range.
    float fragPosXNormalised = (m_fragPos.x + 1) / 2;
    float fragPosYNormalised = (m_fragPos.y + 1) / 2;
    // convert to tile coords.
    int gridX = int(x * u_tilesWide);
    int gridY = int(y * u_tilesHigh);
    // convert x/y to linear index
    int tileIndex = gridX + (gridY * u_tilesWide);
    // texel fetch - read first data item, the light count in this tile.
    vec4 lightCount = texelFetch(u_tileDataTex, ivec2(0, tileIndex), 0);
    // only care about GL_RED, convert red channel from 0-1 to 0-65535
    int numLights = int(0xFFFF * lightCount.x);

    // advance to read light indices
    vec3 lightContribution = vec3(0, 0, 0);
    for (int i = 0; i < numLights; ++i)
    {
        // now reading from the lights array
        // i + 1 because first was the count!
        vec4 lightIndexV4 = texelFetch(u_lightArrayTex, ivec2(i + 1, tileIndex), 0);
        int lightIndex = int(0xFFFF * lightIndex.x);
        // we now fetch light information for light index specified...
        // ...
        // and accumulate this lighting information
        // ...lightContribution += light - distance etc...
    }

    // apply to pixel
    output = diffuse * lightContribution;

	1
-1 		1
	-1
to
	1
0		1
	0
in four tiles:
2 3
0 1
int(0.75 * 2) -> 1
*/
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

struct ZRGLTestLight
{
    Vec3 pos;
    float radius;
    Vec3 colour;
    float strength;
};

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
    const i32 numLights = 3;

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

    // lights
    local_persist ZRGLTestLight lights[numLights];

    // data texture
    local_persist Vec4 g_batchDataPixels[totalDataPixels];
    local_persist GLuint g_dataTextureHandle;
    local_persist GLuint g_samplerDataTex2D;
    local_persist i32 g_dataPixelStride = 2;

    local_persist ZRU16Texture* g_tileData = NULL;
    local_persist i32 g_lightingTilesWidth = 2;
    local_persist i32 g_lightingTilesHeight = 2;

    // lights array
    local_persist GLuint g_viewLightsHandle = 0;
    local_persist ZRVec4Texture* viewLights = NULL;
    const i32 viewSceneLights = 256;
    const i32 dataPixelsPerLight = 2;

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
            draw_sprite_batch_test_4_frag_text,
            "sprite_batch_test_3",
            &g_batchDrawShader);
        ZE_ASSERT(err == ZE_ERROR_NONE, "create prog failed")

        g_meshId = ZAssets_GetMeshByName(ZE_EMBEDDED_QUAD_NAME)->header.id;

        GetEngine().input.AddAction(Z_INPUT_CODE_LEFT, 0, "turn_left");
        GetEngine().input.AddAction(Z_INPUT_CODE_RIGHT, 0, "turn_right");

        GetEngine().input.AddAction(Z_INPUT_CODE_SPACE, 0, "move_forward");
        GetEngine().input.AddAction(Z_INPUT_CODE_LEFT_SHIFT, 0, "move_backward");
        GetEngine().input.AddAction(Z_INPUT_CODE_A, 0, "move_left");
        GetEngine().input.AddAction(Z_INPUT_CODE_D, 0, "move_right");

        GetEngine().input.AddAction(Z_INPUT_CODE_W, 0, "move_up");
        GetEngine().input.AddAction(Z_INPUT_CODE_S, 0, "move_down");

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
            g_batchDataPixels[pixel] =
                {
                    RANDF_RANGE(-rangeX, rangeX),
                    RANDF_RANGE(-rangeY, rangeY),
                    0,//RANDF_RANGE(-rangeZ, rangeZ),
                    RANDF_RANGE(0, 360) * DEG2RAD
                };
            g_batchDataPixels[pixel + 1] = uvs[i % 4];
            g_batchDataPixels[pixel + 2] = { RANDF_RANGE(0.1f, 1), RANDF_RANGE(0.1f, 1) };
        }
        #endif

        g_instanceCount = i;
        printf("Drawing %d instances of %d max\n", g_instanceCount, batchMax);
        printf("\tData texture is %lldKB\n", (totalDataPixels * sizeof(Vec4)) / 1024);

        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, dataTextureSize, dataTextureSize,
            GL_RGBA, GL_FLOAT, g_batchDataPixels);
        CHECK_GL_ERR

        // Samplers for data textures
        // Make sure filtering and mip-mapping are disabled!
        glGenSamplers(1, &g_samplerDataTex2D);
        CHECK_GL_ERR
        glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        CHECK_GL_ERR
        glSamplerParameteri(g_samplerDataTex2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        CHECK_GL_ERR

        /////////////////////////////////////////////////////////////////////////
        // setup tile data
        
        g_tileData = U16Tex_Alloc(256, 256);
        i32 totalTilesCells = 256 * 256;
        for (i = 0; i < totalTilesCells; ++i)
        {
            g_tileData->data[i] = 255;
        }
        /*
        2, 3
        0, 1
        */
        // set light counts
        g_tileData->data[ZE_2D_INDEX(0, 0, 256)] = 2; //0xFFFF;
        g_tileData->data[ZE_2D_INDEX(0, 1, 256)] = 1; //0xFFFF / 2;
        g_tileData->data[ZE_2D_INDEX(0, 2, 256)] = 0; //0x7FFF / 3;
        g_tileData->data[ZE_2D_INDEX(0, 3, 256)] = 1; //0x7FFF / 4;

        // give tile 0 some light indices
		// x coord is the data column (0 == count, 1 and up are light indices)
		// y coord is the tile index
		// first light
        g_tileData->data[ZE_2D_INDEX(1, 0, 256)] = 2; // light 2 (blue)
		// second light
		g_tileData->data[ZE_2D_INDEX(2, 0, 256)] = 1; // light 1 (green)
		
		// tile 1
		g_tileData->data[ZE_2D_INDEX(1, 1, 256)] = 1; // light 1 (green)
		
		// tile 3
		g_tileData->data[ZE_2D_INDEX(1, 3, 256)] = 0; // light 0 (red)
    
        // upload tile data
        glGenTextures(1, &g_tileData->handle);
        CHECK_GL_ERR
        glBindTexture(GL_TEXTURE_2D, g_tileData->handle);
        CHECK_GL_ERR

        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
        //     g_tileData->header.width, g_tileData->header.height,
        //     0, GL_RED, GL_UNSIGNED_SHORT, g_tileData->data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI,
            g_tileData->header.width, g_tileData->header.height,
            0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, g_tileData->data);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
        //     g_tileData->header.width, g_tileData->header.height,
        //     0, GL_RGBA, GL_FLOAT, g_tileData->data);
        CHECK_GL_ERR

        printf("Created tile data tex %d by %d, handle %d\n",
            g_tileData->header.width, g_tileData->header.height, g_tileData->handle);
        
        // setup light data
        lights[0].pos = { 4, 4, 0 };
        lights[0].radius = 3;
        lights[0].colour = { 1, 0, 0 };
        lights[0].strength = 1;

        lights[1].pos = { -4, 4, 0 };
        lights[1].radius = 6;
        lights[1].colour = { 0, 1, 0 };
        lights[1].strength = 1;

        lights[2].pos = { -4, -4, 0 };
        lights[2].radius = 5;
        lights[2].colour = { 0, 0, 1 };
        lights[2].strength = 1;

        // Create texture with light data here...
        viewLights = Vec4Tex_Alloc(dataPixelsPerLight, viewSceneLights);
        Vec4Tex_SetAll(viewLights, { 0.5f, 0.5f, 0.5f, 0.5f });
        
        // write lights array data
        i32 lightNumber = 0;
        i = ZE_2D_INDEX(0, lightNumber, dataPixelsPerLight);
        viewLights->data[i] = { -4, -4, 0, 1 };
        viewLights->data[i + 1] = { 1, 0, 0, 1 };
        
        lightNumber++;
        i = ZE_2D_INDEX(0, lightNumber, dataPixelsPerLight);
        viewLights->data[i] = { -4, 4, 0, 1 };
        viewLights->data[i + 1] = { 0, 1, 0, 1 };
        
        lightNumber++;
        i = ZE_2D_INDEX(0, lightNumber, dataPixelsPerLight);
        viewLights->data[i] = { 4, 4, 0, 1 };
        viewLights->data[i + 1] = { 0, 0, 1, 1 };

        g_viewLightsHandle = Vec4Tex_Register(viewLights);

        
        /*
        Set up for drawing debug quad
        */
        err = ZRGL_CreateProgram(
            sandbox_draw_single_mesh_vert_text,
            sandbox_draw_screen_tiles_frag_text,
            "draw_tiles_debug",
            &g_blendShader);
        g_blendShader.flags = ZR_SHADER_FLAG_BLEND | ZR_SHADER_FLAG_NO_DEPTH;
    }
#if 1
    
    
    
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////
    // execute
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////


    // calculate delta
    f64 now = Platform_QueryClock();
    f32 delta = (f32)(now - g_lastTickTime);
    g_lastTickTime = now;

#if 0 // update data texture to check data refreshing
    for (i32 i = 0; i < g_instanceCount; i += 1)
    {
        i32 pixel = g_dataPixelStride * i;
        g_batchDataPixels[pixel] =
        {
            RANDF_RANGE(-range, range),
            RANDF_RANGE(-range, range),
            RANDF_RANGE(-4, -16),
            1
        };
        g_batchDataPixels[pixel + 1] = { 0.75f, 0.25f, 0.75f, 0.25f };
    }
    glBindTexture(GL_TEXTURE_2D, g_dataTextureHandle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
    dataTextureSize, dataTextureSize,  GL_RGBA, GL_FLOAT, g_batchDataPixels);
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

	ZR_SetProg1i(g_batchDrawShader.handle, "u_tilesWide", g_lightingTilesWidth);
    ZR_SetProg1i(g_batchDrawShader.handle, "u_tilesHigh", g_lightingTilesHeight);
	ZR_PrepareTextureUnit2D(
        g_batchDrawShader.handle, GL_TEXTURE0, 0, "u_tileDataTex", g_tileData->handle, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        g_batchDrawShader.handle, GL_TEXTURE2, 2, "u_lightsArrayTex", g_viewLightsHandle, g_samplerDataTex2D);

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
    if (engine.input.GetActionValue("move_up")) { move.y += 1; }
    if (engine.input.GetActionValue("move_down")) { move.y -= 1; }

    Vec3 moveDir = M3x3_Calculate3DMove(&camera.rotation, move);
    Vec3_MulF(&moveDir, 10.f * delta);
    Vec3_AddTo(&camera.pos, moveDir);

    if (engine.input.GetActionValue("move_forward"))
    { g_orthographicSize -= 1.f * delta; }
    if (engine.input.GetActionValue("move_backward"))
    { g_orthographicSize += 1.f * delta; }

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





    //////////////////////////////////////////////////////////
    // draw some cubes to represent lights and their area of influence
    ZRDrawCmdMesh cubeCmd = {};
    Transform_SetToIdentity(&cubeCmd.obj.t);
    i32 cubeMeshIndex = ZAssets_GetMeshByName(ZE_EMBEDDED_CUBE_NAME)->header.id;
    i32 cubeMaterialIndex = ZAssets_GetMaterialByName(FALLBACK_CHEQUER_MATERIAL)->header.id;
    // i32 cubeMaterialIndex = ZAssets_GetMaterialByName("white")->header.id;
    cubeCmd.obj.data.SetAsMesh(cubeMeshIndex, cubeMaterialIndex);
	
	for (i32 i = 0; i < numLights; ++i)
	{
        ZRGLTestLight* light = &lights[i];
		cubeCmd.obj.t.pos = light->pos;
		cubeCmd.obj.t.scale = { light->radius, light->radius, 1 };
		ZRGL_DrawMesh(&cubeCmd, &view, &projection);
	}



    //////////////////////////////////////////////////////////
    #if 1 // draw overlay
    // draw tiles overlay
    ZR_PrepareShader(&g_blendShader);

    ZR_SetProg1i(g_blendShader.handle, "u_tilesWide", g_lightingTilesWidth);
    ZR_SetProg1i(g_blendShader.handle, "u_tilesHigh", g_lightingTilesHeight);

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
    ZR_PrepareTextureUnit2D(
        g_blendShader.handle, GL_TEXTURE0, 0, "u_tileDataTex", g_tileData->handle, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        g_blendShader.handle, GL_TEXTURE2, 2, "u_lightsArrayTex", g_viewLightsHandle, g_samplerDataTex2D);
    // Vec4 colour = { 0.5f, 0.5f, 0.5f, 0.5f };
    Vec4 colour = { 1, 0.5f, 0.5f, 0.5f };
    // ZR_SetProgVec4f(g_blendShader.handle, "u_colour", colour);
    glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount);
    
    #endif // draw overlay
#endif
}

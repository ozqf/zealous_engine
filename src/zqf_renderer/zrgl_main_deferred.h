#ifndef ZRGL_MAIN_DEFERRED_H
#define ZRGL_MAIN_DEFERRED_H

#include "zrgl_internal.h"

#if 0
static void ZR_DrawDeferredLight(
    ZRGBuffer* gBuf, Transform* cameraTrans, Transform* lightTrans, ScreenInfo* scrInfo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Vec3 pos = cameraTrans->pos;
    /////////////////////////////////////////////////////////
    // set program and geometry

    ZRPrefab* prefab = &g_prefabs[ZR_PREFAB_TYPE_SPHERE];
    //ZRPrefab* prefab = &g_prefabs[ZR_PREFAB_TYPE_CUBE];
	glBindVertexArray(prefab->geometry.vao);
    
    /////////////////////////////////////////////////////////
    // Prepare FS params
    M4x4_CREATE(projection)
    M4x4_CREATE(view)
    M4x4_CREATE(model)
    M4x4_CREATE(modelView)


    // build projection
    COM_SetupDefault3DProjection(projection.cells, scrInfo->aspectRatio);
    // Build model and view
    ZR_BuildModelMatrix(&model, lightTrans);
    ZR_BuildViewMatrix(&view, cameraTrans);
    // Combine
    M4x4_SetToIdentity(modelView.cells);
    M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
    M4x4_Multiply(modelView.cells, model.cells, modelView.cells);

    /////////////////////////////////////////////////////////
    // prepare basic 
    /////////////////////////////////////////////////////////
    GLint prog = g_programs[ZR_SHADER_TYPE_BLOCK_COLOUR].handle;
    glUseProgram(prog);
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
    ZR_SetProgVec3f(prog, "u_colour", { 1, 1, 1 });
    
    /////////////////////////////////////////////////////////
    // Draw stencil
    glDrawBuffer(GL_NONE);
    glDrawArrays(GL_TRIANGLES, 0, prefab->geometry.vertexCount);
    CHECK_GL_ERR
    glDrawBuffer(GL_BACK);
    
    /////////////////////////////////////////////////////////
    // upload FS params
    prog = g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_VOLUME].handle;
    glUseProgram(prog);

    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
    ZR_SetProgM4x4(prog, "u_model", model.cells);

    ZR_SetProg1i(prog, "u_windowWidth", scrInfo->width);
    ZR_SetProg1i(prog, "u_windowHeight", scrInfo->height);
    
    /////////////////////////////////////////////////////////
    // upload gbuffer params

    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_colourTex", gBuf->colourTex, g_samplerDataTex2D);
    
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_normalTex", gBuf->normalTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE2, 2, "u_positionTex", gBuf->positionTex, g_samplerDataTex2D);
    
    /////////////////////////////////////////////////////////
    // Draw Light
    glDrawArrays(GL_TRIANGLES, 0, prefab->geometry.vertexCount);
    CHECK_GL_ERR
}

static void ZR_DrawDeferredVolumeLights(ZRGBuffer* gBuf, Transform* camera, ScreenInfo* scrInfo)
{
    //////////////////////////////////////////////
    // configure state
    
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    // Disable depth and allow backface drawing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    // enable blending for each light to add result
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glStencilFunc(GL_ALWAYS, 0, 0);

    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

    //////////////////////////////////////////////
    // Draw lights
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glDepthMask(GL_FALSE);

    Transform light;
    Transform_SetToIdentity(&light);
    light.scale = { 4, 4, 4 };

    light.pos = { 4, 0, 0 };
    ZR_DrawDeferredLight(&g_gBuffer, camera, &light, scrInfo);

    light.pos = { -4, 0, 0 };
    ZR_DrawDeferredLight(&g_gBuffer, camera, &light, scrInfo);

    //////////////////////////////////////////////
    // clean up
    glDepthMask(GL_TRUE);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
#endif

static ZRGroupingStats ZR_PrepareSceneDeferred(
    ZRSceneFrame* sceneCmd, ZEByteBuffer* scratch, ScreenInfo scrInfo)
{
    ZRGroupingStats stats = {};
    f64 start = g_platform.QueryClock();

    M4x4* projection = &sceneCmd->drawTime.projection;
    // Setup
    if (sceneCmd->params.projectionMode == ZR_PROJECTION_MODE_IDENTITY)
    {
        M4x4_SetToIdentity(projection->cells);
    }
    else
    {
        COM_SetupDefault3DProjection(projection->cells,
            scrInfo.aspectRatio);
    }
    M4x4_CREATE(camera)
    Transform_ToM4x4(&sceneCmd->params.camera, &camera);
    // Group objects
    // TODO: Urrgh ugly:
    sceneCmd->drawTime.objects = (ZRDrawObj*)(((u8*)sceneCmd) + sizeof(ZRSceneFrame));
    ZRDrawObj* objects = sceneCmd->drawTime.objects;

    ///////////////////////////////////////////////////////////
    // Build Groups
    sceneCmd->drawTime.view = ZR_BuildDrawGroups(
        objects, sceneCmd->params.numObjects, scratch, &stats);
    
    ZRSceneView* view = sceneCmd->drawTime.view;
    stats.numGroups = view->numGroups;
    stats.numLights = view->numLights;

    ///////////////////////////////////////////////////////////
    // Deferred Geometry pass
    f64 gBufStart = g_platform.QueryClock();
        ZRGL_FillGBuffer(
            &g_gBuffer,
            &sceneCmd->params.camera,
            scrInfo,
            view->groups,
            view->numGroups,
            objects,
            sceneCmd->params.numObjects,
            &stats);
        stats.gBufferTime = (g_platform.QueryClock() - gBufStart) * 1000;

    ///////////////////////////////////////////////////////////
    // draw lights
    
    // disable depth testing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    // enable blending
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    


    for (i32 i = 0; i < view->numLights; ++i)
    {
        i32 lightObjIndex = view->lights[i];
        ZRDrawObj* lightObj = &sceneCmd->drawTime.objects[lightObjIndex];
        // TODO: Grouped on point light here. no way to put direct lights in
        Vec3 dir = lightObj->t.rotation.zAxis;
        f32 multiplier = lightObj->data.light.settings.x;
        f32 range = lightObj->data.light.settings.y;
        Colour c = lightObj->data.light.colour;
        Vec3 pos = lightObj->t.pos;
        printf("ZRGL Draw light obj multiplier pos %.3f, %.3f, %.3f - %.3f, range %.3f\n",
            pos.x, pos.y, pos.z, multiplier, range);
        ZRGL_GBufferDrawPointLight(
            &g_gBuffer,
            lightObj->t.pos,
            dir,
            { c.r, c.g, c.b },
            multiplier,
            range);
    
    }

    glDisable(GL_BLEND);

    // reenable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);

    f64 end = g_platform.QueryClock();
    stats.time = end - start;
    return stats;
}

///////////////////////////////////////////////////////////
// Frame draw entry point
///////////////////////////////////////////////////////////
static ZRPerformanceStats ZRImpl_DrawFrameDeferred(
    ZEByteBuffer* drawList,
    ZEByteBuffer* drawData,
    ScreenInfo scrInfo)
{
    ZRPerformanceStats stats = {};
    if (Buf_IsValid(drawList) == NO) { return stats; }
    if (Buf_IsValid(drawData) == NO) { return stats; }
	// This will almost always happen
	// whilst the app thread starts up
	i32 listBytes = drawList->Written();
	if (listBytes == 0)
	{
		return stats;
	}

    if (g_bDrawLocked == YES)
    {
        ILLEGAL_CODE_PATH
    }
    g_bDrawLocked = YES;

    stats.listBytes = drawList->Written();
    stats.dataBytes = drawData->Written();

    // Reset frame scratch memory cursor
    g_scratch.Clear(NO);

    // Reset data texture
    g_dataTex2D.cursor = 0;

    u8* cursor = drawList->start;
    u8* end = drawList->cursor;

    ZRViewFrame* header = (ZRViewFrame*)cursor;
    ZE_ASSERT(header->sentinel == ZR_SENTINEL, "Sentinel check failed")
    cursor += sizeof(ZRViewFrame);
    u8* scenesStart = cursor;

	/////////////////////////////////////////////////////////////
    // Clear
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR
    glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERR
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERR

    /////////////////////////////////////////////////////////////
    // Prepare scenes
    ZRSceneFrame* firstScene = (ZRSceneFrame*)cursor;
    cursor += sizeof(ZRSceneFrame) + firstScene->params.dataBytes;
    ZRGroupingStats result = ZR_PrepareSceneDeferred(firstScene, &g_scratch, scrInfo);

    // Group objects
    /*f64 prepareStart = g_platform.QueryClock();
    for (i32 i = 0; i < header->numScenes; ++i)
    {
        ZRSceneFrame* cmd = (ZRSceneFrame*)cursor;
        ZE_ASSERT(cmd->sentinel == ZR_SENTINEL,
            "Scene cmd sentinel check failed")
        if (firstScene == NULL) { firstScene = cmd; }
        cursor += sizeof(ZRSceneFrame) + cmd->params.dataBytes;
        // writes geometry to gbuffer:
		ZRGroupingStats result = ZR_PrepareSceneDeferred(cmd, &g_scratch, scrInfo);
        if (cmd->params.bIsInteresting)
        {
            stats.grouping = result;
        }
    }*/

    // TODO: Move this into scene specific stuff:
    // take camera from first scene:
    Transform* cam = &firstScene->params.camera;

    // draw skybox
    //ZR_DrawSkybox(&firstScene->drawTime.projection, cam);

    // Draw lights
    #if 0
    ZR_DrawDeferredVolumeLights(&g_gBuffer, cam, &scrInfo);
    #endif
    
    // Draw gbuffer debug result to screen
    //ZRGL_DrawDebugGBufferCombine(&g_gBuffer);
    #if 0
    // disable depth testing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    // enable blending
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    
    M3x3_CREATE(rot);
    M3x3_RotateX(rot.cells, -45 * DEG2RAD);
    M3x3_RotateY(rot.cells, 45 * DEG2RAD);
    // Draw gbuffer debug result to screen
    Vec3 lightRed = { 1, 0, 0 };
    Vec3 lightGreen = { 0, 1, 0 };
    Vec3 lightBlue = { 0, 0, 1 };
    Vec3 lightYellow = { 1, 1, 0 };
    Vec3 white = { 1, 1, 1 };
    ZRGL_GBufferDrawDirectLight(
        &g_gBuffer,
        { 0, 5, 0 },
        rot.zAxis,
        white, 0.4f, 25);
    
    ZRGL_GBufferDrawPointLight(
        &g_gBuffer,
        { 15, 5, 15 },
        { 0, -1, 0 },
        lightRed, 2, 25);
    
    ZRGL_GBufferDrawPointLight(
        &g_gBuffer,
        { -15, 5, -15 },
        { 0, -1, 0 },
        lightGreen, 2, 25);
    
    ZRGL_GBufferDrawPointLight(
        &g_gBuffer,
        { -15, 5, 15 },
        { 0, -1, 0 },
        lightBlue, 2, 25);
    
    ZRGL_GBufferDrawPointLight(
        &g_gBuffer,
        { 15, 5, -15 },
        { 0, -1, 0 },
        lightYellow, 2, 25);
    
    glDisable(GL_BLEND);

    // reenable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    #endif
    // Draw debug cack
    #if 1
    ZRGL_DrawGBufferDebugQuads(scrInfo.aspectRatio);
    #endif

    /////////////////////////////////////////////////////////////
    // Done. Gather stats
    g_bDrawLocked = NO;
    
    f64 drawEnd = g_platform.QueryClock();
    /*
    stats.prepareTime = (prepareEnd - prepareStart) * 1000;
	stats.uploadTime = (uploadEnd - uploadStart) * 1000;
	stats.drawTime = (drawEnd - drawStart) * 1000;
	stats.total = stats.prepareTime + stats.uploadTime + stats.drawTime;
	i32 texIndex = g_dataTex2D.cursor;
	i32 totalPixels = g_dataTex2D.width * g_dataTex2D.height;
    stats.dataTexPercentUsed = ((f32)texIndex / (f32)totalPixels) * 100.f;
    */
    return stats;
}

#endif // ZRGL_MAIN_DEFERRED_H
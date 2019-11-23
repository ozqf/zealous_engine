#ifndef ZRGL_MAIN_DEFERRED_H
#define ZRGL_MAIN_DEFERRED_H

#include "zrgl_internal.h"

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
    // draw shadow maps
    #if 1
    for (i32 i = 0; i < view->numLights; ++i)
    {
        i32 lightObjIndex = view->lights[i];
        ZRDrawObj* lightObj = &sceneCmd->drawTime.objects[lightObjIndex];
        ZE_ASSERT(lightObj->type == ZR_DRAWOBJ_TYPE_LIGHT,
            "Object in light list is not a light!")
        
        if (lightObj->data.light.bCastShadows)
        {
            #if 1
            ZRGL_WriteTestShadowMap(
                //&sceneCmd->params.camera,
                &lightObj->t,
                scrInfo,
                view->groups,
                view->numGroups,
                objects,
                sceneCmd->params.numObjects,
                &stats);
            #endif
            #if 1 // Debug, draw scene to colour texture
            ZRGL_DrawSceneToTexture(
                //&sceneCmd->params.camera,
                &lightObj->t,
                scrInfo,
                view->groups,
                view->numGroups,
                objects,
                sceneCmd->params.numObjects,
                &stats);
            #endif
        }
    }
    #endif

    ///////////////////////////////////////////////////////////
    // Deferred
    if (sceneCmd->params.bDeferred)
    {
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
    }

    f64 end = g_platform.QueryClock();
    stats.time = end - start;
    return stats;
}

static void ZR_DrawDeferredLight(
    ZRGBuffer* gBuf, Transform* cameraTrans, Transform* lightTrans, ScreenInfo* scrInfo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Vec3 pos = cameraTrans->pos;
    /////////////////////////////////////////////////////////
    // set program and geometry

    GLint prog = g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT].handle;
    glUseProgram(prog);
    ZRPrefab* prefab = &g_prefabs[ZR_PREFAB_TYPE_CUBE];
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
    // upload FS params
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
    ZR_SetProgM4x4(prog, "u_model", model.cells);
    
    /////////////////////////////////////////////////////////
    // upload gbuffer params

    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_colourTex", gBuf->colourTex, g_samplerDataTex2D);
    
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_normalTex", gBuf->normalTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE2, 2, "u_positionTex", gBuf->positionTex, g_samplerDataTex2D);
    
    glDrawArrays(GL_TRIANGLES, 0, prefab->geometry.vertexCount);
    CHECK_GL_ERR
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
	glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERR
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERR

    /////////////////////////////////////////////////////////////
    // Prepare scenes
    ZRSceneFrame* firstScene = NULL;

    // Group objects
    f64 prepareStart = g_platform.QueryClock();
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
    }

    // Draw lights
    // take camera from first scene:
    Transform* cam = &firstScene->params.camera;
    Transform light;
    Transform_SetToIdentity(&light);
    light.scale = { 4, 4, 4 };
    ZR_DrawDeferredLight(&g_gBuffer, cam, &light, &scrInfo);
    
    // Draw gbuffer result to screen
    //ZRGL_CombineGBuffer(&g_gBuffer);

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
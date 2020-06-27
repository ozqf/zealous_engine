#ifndef ZRGL_MAIN_DEFERRED_H
#define ZRGL_MAIN_DEFERRED_H

#include "zrgl_internal.h"

struct ZR_DeferredStats
{
    f32 geometryMS;
    f32 lightingMS;
    i32 numLights;
};

#if 0
static void ZR_DrawDeferredLight(
    ZRGBuffer* gBuf, Transform* cameraTrans, Transform* lightTrans, ScreenInfo* scrInfo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Vec3 pos = cameraTrans->pos;
    /////////////////////////////////////////////////////////
    // set program and geometry
	
	i32 vao, vertCount;
	// TODO replace with Get Cube or Sphere etc.
	ZRGL_GetQuadHandles(&vao, &vertCount);
	glBindVertexArray(vao);
    
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
    glDrawArrays(GL_TRIANGLES, 0, vertCount);
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
    glDrawArrays(GL_TRIANGLES, 0, vertCount);
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

static ZRGroupingStats ZR_DrawSceneDeferred(
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
    // sceneCmd->drawTime.view = ZR_BuildDrawGroups(
    //     objects, sceneCmd->params.numObjects, scratch, &stats);
    
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
        stats.gBufferFillMS = (g_platform.QueryClock() - gBufStart) * 1000;

    ///////////////////////////////////////////////////////////
    // draw lights
    ///////////////////////////////////////////////////////////
    stats.numLights = view->numLights;
    f64 gBufLightStart = g_platform.QueryClock();
    
    ///////////////////////////////////////////////////////////
    // individual lights mode
    #if 0
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
        f32 multiplier = lightObj->data.pointLight.multiplier;
        f32 range = lightObj->data.pointLight.range;
        Colour c = lightObj->data.pointLight.colour;
        Vec3 pos = lightObj->t.pos;
        
        switch (lightObj->data.type)
        {
            case ZR_DRAWOBJ_TYPE_POINT_LIGHT:
            ZRGL_GBufferDrawPointLight(
                &g_gBuffer,
                lightObj->t.pos,
                dir,
                { c.r, c.g, c.b },
                multiplier,
                range);
            break;

            case ZR_DRAWOBJ_TYPE_DIRECT_LIGHT:
            ZRGL_GBufferDrawDirectLight(
                &g_gBuffer,
                lightObj->t.pos,
                dir,
                { c.r, c.g, c.b },
                multiplier,
                range);
            break;
        }
    
    }
    
    glDisable(GL_BLEND);

    // reenable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);

    stats.gBufferLightMS = (g_platform.QueryClock() - gBufLightStart) * 1000;
    #endif
    
    ///////////////////////////////////////////////////////////
    // NOT WORKING! batched lights mode
    #if 0
    // disable depth testing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    // load light data
    
    for (i32 i = 0; i < view->numLights; ++i)
    {

    }

    // clean up
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    #endif
    
    ///////////////////////////////////////////////////////////
    // no light debug mode
    #if 1
    ZRGL_DrawDebugGBufferCombine(&g_gBuffer);
    #endif

    f64 end = g_platform.QueryClock();
    stats.time = end - start;
    return stats;
}

///////////////////////////////////////////////////////////
// Frame draw entry point
///////////////////////////////////////////////////////////
extern "C" ZRPerformanceStats ZRImpl_DrawFrameDeferred(
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
    stats.numDataBytes = drawData->Written();

    // Reset frame scratch memory cursor
    g_scratch.Clear(NO);
    // allocate some space in scratch for debug string
    ZEByteBuffer debugStr = Buf_SubBuffer(&g_scratch, KiloBytes(4));
    if (debugStr.capacity == 0)
    {
        printf("Failed to allocate space for debug str\n");
    }

    // Reset data texture
    g_dataTex2D.cursor = 0;

    u8* cursor = drawList->start;
    u8* end = drawList->cursor;

    /////////////////////////////////////////////////////////////
    // K start
    ZRViewFrame* header = (ZRViewFrame*)cursor;
    
    if (header->bVerbose == YES)
    {
        printf("ZRGL - Verbose frame!\n");
        g_verboseFrame = YES;
    }
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
    // Draw main scene
    ZRSceneFrame* firstScene = (ZRSceneFrame*)cursor;
    cursor += sizeof(ZRSceneFrame) + firstScene->params.numDataBytes;
    ZRGroupingStats gBufStats = ZR_DrawSceneDeferred(firstScene, &g_scratch, scrInfo);

    /////////////////////////////////////////////////////////////
    // Additional forward render calls

    // TODO: Move this into scene specific stuff:
    // take camera from first scene:
    Transform* cam = &firstScene->params.camera;

    // draw skybox
    //ZR_DrawSkybox(&firstScene->drawTime.projection, cam);

    // Draw lights
    #if 0
    ZR_DrawDeferredVolumeLights(&g_gBuffer, cam, &scrInfo);
    #endif
    
    // Draw debug cack
    #if 0
    ZRGL_DrawGBufferDebugQuads(scrInfo.aspectRatio);
    #endif

    /////////////////////////////////////////
    // Draw debug text
    /////////////////////////////////////////
    
    // allocate space in scratch for debug string
    #if 1
    i32 written = sprintf_s((char*)debugStr.cursor, debugStr.Space(),
        "Prebuild: %.3fMS\nObj List %dKB\nObj Data %dKB\nGBuffer Fill %.3fMS\nGBuffer Light %.3fMS\nNum lights: %d\nSwapMS %.3f\nTotalMS %.3f\n",
        header->prebuildTime * 1000,
        drawList->Written() / 1024,
        drawData->Written() / 1024,
        gBufStats.gBufferFillMS,
        gBufStats.gBufferLightMS,
        gBufStats.numLights,
        g_platformSwapMS * 1000,
        g_platformFrameMS * 1000);
    debugStr.cursor += written;
    
    f32 screenSpaceHeight = 2;
    f32 numLinesInScreen = 64;
    ZRDrawCmd_Text txtCmd = {};
    txtCmd.origin = { -1, 1 }; // screen topleft
    txtCmd.numChars = written;// strlen(testText);
    txtCmd.charSize = screenSpaceHeight / numLinesInScreen;
    txtCmd.aspectRatio = scrInfo.aspectRatio;
    txtCmd.offsetToString = 0; // TOOD: Remove
    txtCmd.alignmentMode = 0;
    M4x4_CREATE(textProjection);
    //printf("Draw %d debug chars\n", txtCmd.numChars);
    ZR_ExecuteTextDraw(&txtCmd, &textProjection, (char*)debugStr.start, &stats);
    #endif

    /////////////////////////////////////////////////////////////
    // Done. Gather stats
    g_bDrawLocked = NO;
    g_verboseFrame = NO;
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
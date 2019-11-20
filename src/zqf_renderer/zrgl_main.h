#ifndef ZRGL_MAIN_H
#define ZRGL_MAIN_H

#include "zrgl_internal.h"

static void ZRGL_DrawSceneToTexture(
    Transform* camera, // Object casting light
	ScreenInfo scrInfo,
	ZRDrawGroup** groups, // objects/geometry to draw
	i32 numGroups,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats);

///////////////////////////////////////////////////////////
// Upload/Data for draw calls
///////////////////////////////////////////////////////////
static void ZR_UploadDataTexture()
{
    ZRDataTexture* dataTex2D = &g_dataTex2D;
    
    Vec4* dataPixel = (Vec4*)dataTex2D->mem;
    // upload
    i32 w = dataTex2D->width;
	i32 h = dataTex2D->height;
    
	glBindTexture(GL_TEXTURE_2D, g_dataTex2D.handle);
	CHECK_GL_ERR
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, dataTex2D->mem);
    CHECK_GL_ERR
}

///////////////////////////////////////////////////////////
// Prepare scene - grouping, shadow maps, buffer data etc
// For forward mode: object <-> light interactions
// For deferred: build gbuffer.
///////////////////////////////////////////////////////////
static ZRGroupingStats ZR_PrepareScene(
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

    ///////////////////////////////////////////////////////////
    // locate programs
    for (i32 i = 0; i < view->numGroups; ++i)
    {
        ZRDrawGroup* group = view->groups[i];
        if (group->id.program >= 0 || group->id.program < ZR_SHADER_TYPE_LAST__)
        {
            group->shader = &g_programs[group->id.program];
            if (group->id.objType == group->shader->drawObjType)
            {
                // This group <-> program is okay.
                continue;
            }
        }

        // Config of this group is wrong, ignore it
        group->shader = NULL;
        
    }

    // Write data texture
    ZR_WriteGroupsToTextureByIndex(
        objects, sceneCmd->params.numObjects, &sceneCmd->params.camera,
        view, &g_dataTex2D);

    f64 end = g_platform.QueryClock();
    stats.time = end - start;
    return stats;
}

///////////////////////////////////////////////////////////
// Draw Scene
///////////////////////////////////////////////////////////

static void ZRGL_DrawSceneToTexture(
    Transform* camera, // Object casting light
	ScreenInfo scrInfo,
	ZRDrawGroup** groups, // objects/geometry to draw
	i32 numGroups,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats)
{
    glClearColor(0, 1, 0, 1);
    ZRFrameBuffer* fb = &g_rendToTexFB;
    stats->shadowMaps++;
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    
    CHECK_GL_ERR
    glViewport(0, 0, ZR_SHADOW_MAP_WIDTH, ZR_SHADOW_MAP_HEIGHT);
    CHECK_GL_ERR
	glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERR
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERR
	
	// Draw
    for (i32 i = 0; i < numGroups; ++i)
    {
        ZRDrawGroup* group = groups[i];
        // filter objects that will not cast shadows
        if (group->id.objType != ZR_DRAWOBJ_TYPE_MODEL) { continue; }
        #if 1 // write colour texture
        ZR_DrawMeshGroupTest(
            camera,
            group,
            objects,
            numObjects,
            &scrInfo,
            stats
        );
        #endif
    }
    
	// Restore previous settings
	glViewport(0, 0, scrInfo.width, scrInfo.height);
    CHECK_GL_ERR
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR
    ZRGL_ClearColourDefault();
}


static void ZR_DrawScene(
    ZRSceneFrame* sceneCmd,
    ZEByteBuffer* scratch,
    ScreenInfo scrInfo,
    ZRPerformanceStats* stats)
{
    glViewport(0, 0, scrInfo.width, scrInfo.height);
    // draw
    #if 1
    if (sceneCmd->params.bSkybox == YES)
    {
        ZR_DrawSkybox(&sceneCmd->drawTime.projection, &sceneCmd->params.camera);
    }
    #endif
    #if 1
    for (i32 i = 0; i < sceneCmd->drawTime.view->numGroups; ++i)
    {
        ZRDrawGroup* group = sceneCmd->drawTime.view->groups[i];
        if (group->shader == NULL) { continue; }
        ZR_DrawGroup(
            &sceneCmd->params.camera,
            sceneCmd->drawTime.objects,
            sceneCmd->params.numObjects,
            &sceneCmd->drawTime.projection,
            group,
            &scrInfo,
            stats);
    }
    #endif
}

static void ZRGL_DrawDebugQuads(f32 aspectRatio)
{
    f32 debugQuadSize = 0.5f;
    f32 debugQuadPosOuter = 0.75f;
    f32 debugQuadPosInner = 0.25f;
    #if 1 // Draw render texture debug A
    i32 texA = g_gBuffer.positionTex;
    i32 texB = g_gBuffer.normalTex;
    i32 texC = g_gBuffer.colourTex;
    //i32 texC = g_rendToTexFB.colourTex;
    i32 texD = g_shadowMapFB.depthTex;
    ZRGL_DrawDebugQuad(
        { -debugQuadPosOuter, -debugQuadPosOuter },
        { debugQuadSize, debugQuadSize },
        { 0, 0 },
        { 1, 1 },
        texA, aspectRatio);
    #endif
    #if 1 // B
    ZRGL_DrawDebugQuad(
        { -debugQuadPosInner, -debugQuadPosOuter },
        { debugQuadSize, debugQuadSize },
        { 0, 0 },
        { 1, 1 },
        texB, aspectRatio);
    #endif
    #if 1 // C
    ZRGL_DrawDebugQuad(
        { debugQuadPosInner, -debugQuadPosOuter },
        { debugQuadSize, debugQuadSize },
        { 0, 0 },
        { 1, 1 },
        texC, aspectRatio);
    #endif
    #if 1 // D
    ZRGL_DrawDebugQuad(
        { debugQuadPosOuter, -debugQuadPosOuter },
        { debugQuadSize, debugQuadSize },
        { 0, 0 },
        { 1, 1 },
        texD, aspectRatio);
    #endif

}

///////////////////////////////////////////////////////////
// Frame draw entry point
///////////////////////////////////////////////////////////
static ZRPerformanceStats ZRImpl_DrawFrame(
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

    // Group objects
    f64 prepareStart = g_platform.QueryClock();
    for (i32 i = 0; i < header->numScenes; ++i)
    {
        ZRSceneFrame* cmd = (ZRSceneFrame*)cursor;
        ZE_ASSERT(cmd->sentinel == ZR_SENTINEL,
            "Scene cmd sentinel check failed")
        cursor += sizeof(ZRSceneFrame) + cmd->params.dataBytes;

		ZRGroupingStats result = ZR_PrepareScene(cmd, &g_scratch, scrInfo);
        if (cmd->params.bIsInteresting)
        {
            stats.grouping = result;
        }
    }

    f64 prepareEnd = g_platform.QueryClock();
    /////////////////////////////////////////////////////////////
    // Upload draw data

    // --- All batch data written by this point! ---
    f64 uploadStart = g_platform.QueryClock();
    #if 1
    ZR_UploadDataTexture();
    #endif
    f64 uploadEnd = g_platform.QueryClock();

    // step back to scene start
    cursor = scenesStart;

	/////////////////////////////////////////////////////////////
    // Draw scenes
    f64 drawStart = g_platform.QueryClock();
    #if 1
    for (i32 i = 0; i < header->numScenes; ++i)
    {
        ZRSceneFrame* cmd = (ZRSceneFrame*)cursor;
        ZE_ASSERT(cmd->sentinel == ZR_SENTINEL,
            "Scene cmd sentinel check failed")
        cursor += sizeof(ZRSceneFrame) + cmd->params.dataBytes;

		ZR_DrawScene(cmd, &g_scratch, scrInfo, &stats);
    }
    #endif

    // Draw debug cack
    #if 1
    ZRGL_DrawDebugQuads(scrInfo.aspectRatio);
    #endif

    #if 1 // draw gbuffer in middle of screen
    ZRGL_CombineGBuffer(&g_gBuffer);
    #endif
    
    /////////////////////////////////////////////////////////////
    // Done. Gather stats
    g_bDrawLocked = NO;
    
    f64 drawEnd = g_platform.QueryClock();
    stats.prepareTime = (prepareEnd - prepareStart) * 1000;
	stats.uploadTime = (uploadEnd - uploadStart) * 1000;
	stats.drawTime = (drawEnd - drawStart) * 1000;
	stats.total = stats.prepareTime + stats.uploadTime + stats.drawTime;
	i32 texIndex = g_dataTex2D.cursor;
	i32 totalPixels = g_dataTex2D.width * g_dataTex2D.height;
    stats.dataTexPercentUsed = ((f32)texIndex / (f32)totalPixels) * 100.f;
    return stats;
}


#endif // ZRGL_MAIN_H
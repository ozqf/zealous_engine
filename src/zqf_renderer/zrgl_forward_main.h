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
        ZE_ASSERT(lightObj->data.type == ZR_DRAWOBJ_TYPE_POINT_LIGHT,
            "Object in light list is not a light!")
        
        if (lightObj->data.pointLight.bCastShadows)
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
    ZRGL_ClearColourDefault();
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
        if (group->data.type != ZR_DRAWOBJ_TYPE_MESH) { continue; }
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
        ZR_DrawGroupForward(
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

#endif // ZRGL_MAIN_H
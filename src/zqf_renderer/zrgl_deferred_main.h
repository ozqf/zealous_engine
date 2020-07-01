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
    ZRSceneFrame *sceneCmd, ZEByteBuffer *scratch, ScreenInfo scrInfo)
{
    ZRGroupingStats stats = {};
    f64 start = g_platform.QueryClock();

    M4x4 *projection = &sceneCmd->drawTime.projection;
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
    sceneCmd->drawTime.objects = (ZRDrawObj *)(((u8 *)sceneCmd) + sizeof(ZRSceneFrame));
    ZRDrawObj *objects = sceneCmd->drawTime.objects;

    ///////////////////////////////////////////////////////////
    // Build Groups
    // sceneCmd->drawTime.view = ZR_BuildDrawGroups(
    //     objects, sceneCmd->params.numObjects, scratch, &stats);

    ZRSceneView *view = sceneCmd->drawTime.view;
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

    // lighting modes 1 and 2 are currently either is SUPER slow or broken
    
    ///////////////////////////////////////////////////////////
    // individual lights mode
    // big quad per light - terrible performance.
    if (g_lightingMode == 1)
    {
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
            ZRDrawObj *lightObj = &sceneCmd->drawTime.objects[lightObjIndex];
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
                    {c.r, c.g, c.b},
                    multiplier,
                    range);
                break;

            case ZR_DRAWOBJ_TYPE_DIRECT_LIGHT:
                ZRGL_GBufferDrawDirectLight(
                    &g_gBuffer,
                    lightObj->t.pos,
                    dir,
                    {c.r, c.g, c.b},
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
    }
    else if (g_lightingMode == 2)
    {
        ///////////////////////////////////////////////////////////
        // NOT WORKING! batched lights mode
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
    }
    else
    {
        ///////////////////////////////////////////////////////////
        // no light debug mode
        ZRGL_DrawDebugGBufferCombine(&g_gBuffer);
    }

    f64 end = g_platform.QueryClock();
    stats.time = end - start;
    return stats;
}

#endif // ZRGL_MAIN_DEFERRED_H
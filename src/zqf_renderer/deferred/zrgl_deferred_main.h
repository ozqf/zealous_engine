#ifndef ZRGL_MAIN_DEFERRED_H
#define ZRGL_MAIN_DEFERRED_H

#include "../zrgl_internal.h"

extern "C" ErrorCode ZRGL_InitDeferred(i32 scrWidth, i32 scrHeight)
{
    g_gBuffer = ZRGL_CreateGBuffer(scrWidth, scrHeight);
    
    /////////////////////////////////////////
    // Programs
    /////////////////////////////////////////
    
    ErrorCode err;

    
    err = ZRGL_CreateProgram(
        gbuffer_create_vert_text,
        gbuffer_create_frag_text,
        "BuildGBuffer",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_shdrBuildGBuffer);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_combine_vert_text,
        gbuffer_combine_frag_text,
        "CombineGBuffer",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_shdrCombineGBuffer);
    if (err != ZE_ERROR_NONE) { return err; }

    err = ZRGL_CreateProgram(
        gbuffer_light_direct_vert_text,
        gbuffer_light_point_frag_text,
        "GBufferLightPoint",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_shdrGBufferPointLight);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_light_direct_vert_text,
        gbuffer_light_direct_frag_text,
        "GBufferLightDirect",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_shdrGBufferDirectLight);
    if (err != ZE_ERROR_NONE) { return err; }
    
    err = ZRGL_CreateProgram(
        gbuffer_light_volume_vert_text,
        gbuffer_light_volume_frag_text,
        "GBufferLightVolume",
        ZR_DRAWOBJ_TYPE_MESH,
        NO,
        &g_shdrGBufferVolumeLight);
    if (err != ZE_ERROR_NONE) { return err; }

    glGenSamplers(1, &g_gBufTextureSampler);
    CHECK_GL_ERR
    glSamplerParameteri(g_gBufTextureSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glSamplerParameteri(g_gBufTextureSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    
    return ZE_ERROR_NONE;
}

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

extern "C" ZRGroupingStats ZR_DrawSceneDeferred(
    ZRSceneFrame *sceneCmd, ZEBuffer *scratch, ScreenInfo scrInfo)
{
    ZRGroupingStats stats = {};
    f64 start = ZRGL_QueryClock();

    M4x4 *projection = &sceneCmd->drawTime.projection;
    // Setup
    if (sceneCmd->params.projectionMode == ZR_PROJECTION_MODE_IDENTITY)
    {
        M4x4_SetToIdentity(projection->cells);
    }
    else if (sceneCmd->params.projectionMode == ZR_PROJECTION_MODE_ORTHO_BASE)
    {
        COM_SetupOrthoProjection(projection->cells, 16, 16.f / 9.f);
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
    f64 gBufStart = ZRGL_QueryClock();
    ZRGL_FillGBuffer(
        &g_gBuffer,
        &sceneCmd->params.camera,
        scrInfo,
        view->groups,
        view->numGroups,
        objects,
        sceneCmd->params.numObjects,
        &stats);
    stats.gBufferFillMS = (ZRGL_QueryClock() - gBufStart) * 1000;

    ///////////////////////////////////////////////////////////
    // draw lights
    ///////////////////////////////////////////////////////////
    stats.numLights = view->numLights;
    f64 gBufLightStart = ZRGL_QueryClock();

    // lighting modes 1 and 2 are currently either is SUPER slow or broken
    
    ///////////////////////////////////////////////////////////
    // individual lights mode
    // big quad per light - terrible performance.
    i32 lightMode = ZRGL_GetLightMode();
    if (lightMode == 1)
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

        stats.gBufferLightMS = (ZRGL_QueryClock() - gBufLightStart) * 1000;
    }
    else if (lightMode == 2)
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

    // Draw emission map
    ZRGL_DrawEmission(scrInfo.aspectRatio);

    f64 end = ZRGL_QueryClock();
    stats.time = end - start;
    return stats;
}

#endif // ZRGL_MAIN_DEFERRED_H
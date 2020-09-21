#ifndef ZRGL_DRAW
#define ZRGL_DRAW

#include "zrgl_internal.h"


////////////////////////////////////////////////////////////
// Draw a crude quad directly into screen space.
////////////////////////////////////////////////////////////
// Note for debugging shadow maps: This guy works okay!
static void ZRGL_DrawDebugQuad(
    Vec2 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax, i32 texHandle, f32 aspectRatio, i32 bTransparent)
{
    glDisable(GL_DEPTH_TEST);
    CHECK_GL_ERR
    ZRMeshHandles mesh;
    AssetDb()->GetMeshHandleByName(AssetDb(), "DynamicQuad", &mesh);
    i32 vao = mesh.vao;
    i32 vbo = mesh.vbo;
    // setup prog
    GLint programId = g_programs[ZR_SHADER_TYPE_SHADOW_MAP_DEBUG].handle;
    glUseProgram(programId);
    CHECK_GL_ERR

    M4x4_CREATE(projection)
    M4x4_CREATE(modelView)
    
    modelView.posX = pos.x;
    modelView.posY = pos.y;
    M4x4_SetScale(modelView.cells, size.x, size.y, 1);
    
    ZR_SetProgM4x4(programId, "u_projection", projection.cells);
    CHECK_GL_ERR
    ZR_SetProgM4x4(programId, "u_modelView", modelView.cells);
    CHECK_GL_ERR
    ZR_SetProg1i(programId, "u_transparent", bTransparent);
    
    // Using Data texture sampler here as it ignores mipmapping
    ZR_PrepareTextureUnit2D(
        programId, GL_TEXTURE0, 0, "u_diffuseTex", texHandle, g_samplerDataTex2D);
    
    // Setup buffers
    // NOTE: This is assuming a non-interleaved geometry layout!
    Vec2 uvs[6];
    uvs[0] = { uvMin.x, uvMin.y };
    uvs[1] = { uvMax.x, uvMin.y };
    uvs[2] = { uvMax.x, uvMax.y };
    uvs[3] = { uvMin.x, uvMin.y };
    uvs[4] = { uvMax.x, uvMax.y };
    uvs[5] = { uvMin.x, uvMax.y };

    // Upload to VAO
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ZRGL_BYTES_FOR_QUAD_VERTS, ZRGL_BYTES_FOR_QUAD_UVS, uvs);
    CHECK_GL_ERR

    glDrawArrays(GL_TRIANGLES, 0, 6);
    CHECK_GL_ERR
    glEnable(GL_DEPTH_TEST);
    CHECK_GL_ERR

    ZRGL_ClearBoundGeometry();
}

///////////////////////////////////////////////////////////
// Skybox
///////////////////////////////////////////////////////////
static void ZR_DrawSkybox(M4x4* projection, Transform* camera)
{
    //M4x4_SetToIdentity(projection->cells);
    //M4x4_SetToIdentity(camera->cells);

    M4x4_CREATE(view);
    ZR_BuildViewMatrix(&view, camera);
    view.posX = 0;
    view.posY = 0;
    view.posZ = 0;

    glDepthMask(GL_FALSE);
    GLint progId = g_programs[ZR_SHADER_TYPE_SKYBOX].handle;
    glUseProgram(progId);
    ZR_SetProgM4x4(progId, "u_projection", projection->cells);
    ZR_SetProgM4x4(progId, "u_view", view.cells);
    glBindVertexArray(g_inverseCubeVAO.vao);
#if 1
    ZR_PrepareTextureUnitCubeMap(
        progId,
        GL_TEXTURE0,
        0,
        "u_cubeMap",
        g_cubemapHandle,
        g_samplerA
    );
#endif
#if 1
    ZR_PrepareTextureUnit2D(
        progId,
        GL_TEXTURE1,
        1,
        "u_debug",
        g_defaultDiffuseHandle,
        g_samplerA
    );
#endif
    //glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemapHandle);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}


///////////////////////////////////////////////////////////
// Setup text draw
///////////////////////////////////////////////////////////
static void ZR_DrawTextGroup(
    ZRDrawObj* objects,
    i32 numObjects,
    M4x4* projection,
    ZRDrawGroup* group,
    ScreenInfo* scrInfo,
    ZRPerformanceStats* stats)
{
    
    if (group->data.type != ZR_DRAWOBJ_TYPE_TEXT)
    {
        printf("Not text!\n");
        return;
    }
    if (g_verboseFrame)
    {
        printf("Draw text %s\n", group->data.text.text);
    }
    #if 1
    for (i32 i = 0; i < group->numItems; ++i)
	{
        i32 objIndex = group->indices[i];
		ZRDrawObj* item = &objects[objIndex];
        if (item->data.type != ZR_DRAWOBJ_TYPE_TEXT)
        {
            printf("DrawObj type is not text!\n");
            continue;
        }
        i32 screenLineCount = item->data.text.linesPerScreen;
        if (screenLineCount <= 0) { screenLineCount = ZR_TEXT_SCREEN_LINE_COUNT; }

        ZRDrawCmd_Text text = {};
        text.origin = item->t.pos;
        text.numChars = 0;
        text.charSize = ZR_SCREEN_SPACE_HEIGHT / (f32)screenLineCount;
        text.aspectRatio = scrInfo->aspectRatio;
        text.offsetToString = 0; // TODO Remove.
        text.alignmentMode = item->data.text.alignment;
        text.fontColour = item->data.text.colour;
        text.bgColour = item->data.text.bgColour;

        ZR_ExecuteTextDraw(&text, projection, item->data.text.text, stats);
	}
    #endif
}

/**
 * Draw a rectangle to centre of screen for debugging
 */
static void ZR_DrawMeshTest()
{
    ZRMeshDrawHandles h = ZRGL_ExtractDrawHandles(AssetDb(), 0, 0);

    if (g_verboseFrame) { printf("Draw mesh test\n"); }
    #if 1
    GLint prog = g_programs[ZR_SHADER_TYPE_FALLBACK].handle;
    glUseProgram(prog);
    CHECK_GL_ERR

    M4x4_CREATE(projection)
    COM_SetupDefault3DProjection(projection.cells, 1);
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    ZR_SetProgVec4f(prog, "u_colour", { 0.5, 0.5, 0.5, 1});

    M4x4_CREATE(model)
    M4x4_CREATE(view)
    M4x4_CREATE(modelView)

    model.posZ = -10;
    
    // Prepare geometry
    glBindVertexArray(h.vao);
	CHECK_GL_ERR
    
    M4x4_SetToIdentity(modelView.cells);
    M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
    M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
    
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
    
    glDrawArrays(GL_TRIANGLES, 0, h.vertCount);
    CHECK_GL_ERR
	#endif
}


///////////////////////////////////////////////////////////
// Quick and ugly draw for debugging
///////////////////////////////////////////////////////////
static void ZR_DrawMeshGroupFallback(
    M4x4* projection,
    Transform* camera,
    ZRDrawGroup* group,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats)
{
    ZRMeshDrawHandles h = ZRGL_ExtractDrawHandles(
        AssetDb(), group->data.model.meshIndex, group->data.model.materialIndex);

    if (g_verboseFrame)
    {
        printf("Draw mesh group fallback - %d objects. mesh %d\n",
            group->numItems, h.vao);
    }
    #if 1
    GLint prog = g_programs[ZR_SHADER_TYPE_FALLBACK].handle;
    glUseProgram(prog);
    CHECK_GL_ERR

    // M4x4_CREATE(projection)
    // COM_SetupDefault3DProjection(projection.cells, 1);
    ZR_SetProgM4x4(prog, "u_projection", projection->cells);
    ZR_SetProgVec4f(prog, "u_colour", { 0.5, 0.5, 0.5, 1});

	ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0,
        "u_diffuseTex", h.diffuseHandle,
        g_samplerA);

    M4x4_CREATE(model)
    M4x4_CREATE(view)
    M4x4_CREATE(modelView)

    ZR_BuildViewMatrix(&view, camera);
    
    // Prepare geometry
    glBindVertexArray(h.vao);
	CHECK_GL_ERR
    for (i32 i = 0; i < group->numItems; ++i)
    {
        i32 objIndex = group->indices[i];
        ZRDrawObj* obj = &objects[objIndex];
        
        ZR_BuildModelMatrix(&model, &obj->t);
        M4x4_SetToIdentity(modelView.cells);
        M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
        M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
        
        ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
        
        glDrawArrays(GL_TRIANGLES, 0, h.vertCount);
        CHECK_GL_ERR
    }
	#endif
}

///////////////////////////////////////////////////////////
// Draw with test shadow map
///////////////////////////////////////////////////////////
static void ZR_DrawMeshGroupTest(
    Transform* camera,
    ZRDrawGroup* group,
    ZRDrawObj* objects,
    i32 numObjects,
    ScreenInfo* scrInfo,
    ZRGroupingStats* stats)
{
    ZRMeshDrawHandles h = ZRGL_ExtractDrawHandles(
        AssetDb(), group->data.model.meshIndex, group->data.model.materialIndex);

    if (g_verboseFrame)
    {
        printf("Draw mesh group test - %d objects. mesh %d\n",
            group->numItems, h.vao);
    }
    #if 1
    GLint prog = g_programs[ZR_SHADER_TYPE_TEST].handle;
    glUseProgram(prog);
    CHECK_GL_ERR

    M4x4_CREATE(projection)
    COM_SetupDefault3DProjection(projection.cells, scrInfo->aspectRatio);
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    ZR_SetProgVec4f(prog, "u_colour", { 0, 0, 1, 1});

    // if Render backface is on, ignore shadow bias
    ZR_SetProg1f(prog, "u_shadowBias", 0.002f);
    //ZR_SetProg1f(prog, "u_shadowBias", 0.f);


    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE3, 3, "u_shadowMap", g_shadowMapFB.depthTex, g_samplerDataTex2D);

    M4x4_CREATE(model)
    M4x4_CREATE(view)
    M4x4_CREATE(modelView)
    M4x4_CREATE(depthMVP)
    M4x4_CREATE(biasMatrix)

    ZR_BuildViewMatrix(&view, camera);
    
    // Converts depthMVP from (-1 to 1) range to (0 to 1)
    M4x4_HomogeneousToUV(biasMatrix.cells);

    ZRShadowCaster* shadow = &g_shadow;

    Vec3 lightDir = camera->rotation.zAxis;
    
    // Prepare geometry
    glBindVertexArray(h.vao);
	CHECK_GL_ERR
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_diffuseTex", h.diffuseHandle, g_samplerA);

    for (i32 i = 0; i < group->numItems; ++i)
    {
        i32 objIndex = group->indices[i];
        ZRDrawObj* obj = &objects[objIndex];
        
        // Build model view
        ZR_BuildModelMatrix(&model, &obj->t);
        M4x4_SetToIdentity(modelView.cells);
        M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
        M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
        ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);

        // lighting info
        Vec3 lightViewSpacePos;
        lightViewSpacePos.x = shadow->worldT.pos.x;
        lightViewSpacePos.y = shadow->worldT.pos.y;
        lightViewSpacePos.z = shadow->worldT.pos.z;
        Vec3 v = Vec3_MultiplyByM4x4(&lightViewSpacePos, view.cells);
        ZR_SetProgVec3f(prog, "u_lightViewPos", v);
        ZR_SetProgVec3f(prog, "u_lightColour", { 1, 1, 1 });
        //ZR_SetProgVec3f(prog, "u_lightDir", );
        

        #if 0 // example: calc a full model-view-projection matrix
        M4x4_Multiply(modelView.cells, projection.cells, modelView.cells);
        M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
        M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
        ZR_SetProgM4x4(prog, "u_mvp", modelView.cells);
        #endif

        // shadow mvp
        M4x4_SetToIdentity(depthMVP.cells);
        M4x4_Multiply(depthMVP.cells, shadow->projection.cells, depthMVP.cells);
        M4x4_Multiply(depthMVP.cells, shadow->view.cells, depthMVP.cells);
        M4x4_Multiply(depthMVP.cells, model.cells, depthMVP.cells);
        // bias to 0-1 coords
        //M4x4_Multiply(depthMVP.cells, biasMatrix.cells, depthMVP.cells);
        ZR_SetProgM4x4(prog, "u_depthMVP", depthMVP.cells);

        glDrawArrays(GL_TRIANGLES, 0, h.vertCount);
        CHECK_GL_ERR
        stats->drawCallsShadows++;
    }
	#endif
}

static void ZR_DrawMeshGroupBatched(
    M4x4* projection, ZRDrawGroup* group, ZRPerformanceStats* stats)
{
    if (g_verboseFrame)
    {
        printf("ZR Batch draw. %d pixels per item. %d objects. data offset at %d\n",
            group->pixelsPerItem, group->numItems, group->dataPixelIndex);
    }
    #if 1
    // setup program
    GLuint programId = g_programs[ZR_SHADER_TYPE_BATCHED].handle;
    glUseProgram(programId);

    ZR_SetProgM4x4(programId, "u_projection", projection->cells);

    ZR_SetProg1i(programId, "u_pixelsPerBatchItem",
        group->pixelsPerItem);
    ZR_SetProg1i(programId, "u_dataTexWidth", ZQF_GL_DATA_TEXTURE_WIDTH);
    ZR_SetProg1i(programId, "u_batchOffsetIndex", group->dataPixelIndex);
    ZR_SetProg1i(programId, "u_numLights", ZR_MAX_POINT_LIGHTS_PER_MODEL);

    // Prepare geometry
    //ZRPrefab* obj = ZRGL_GetPrefab(group->data.prefab.prefabId);
    ZRMeshDrawHandles h = ZRGL_ExtractDrawHandles(
        AssetDb(), group->data.model.meshIndex, group->data.model.materialIndex);

    glBindVertexArray(h.vao);
	CHECK_GL_ERR

    // Setup object textures
    ZR_PrepareTextureUnit2D(
        programId, GL_TEXTURE0, 0,
        "u_diffuseTex", h.diffuseHandle,
        g_samplerA);
    #if 0
    ZR_PrepareTextureUnit2D(
        programId, GL_TEXTURE1, 1,
        "u_lightmap", obj->textures.occlusion,
        g_samplerB);
    #endif
    // Data texture
    ZR_PrepareTextureUnit2D(
        programId, GL_TEXTURE2, 2,
        "u_dataTex2D", g_dataTex2D.handle,
        g_samplerDataTex2D);
    
	glDrawArraysInstanced(GL_TRIANGLES, 0, h.vertCount, group->numItems);
    u32 tris = (h.vertCount / 3) * group->numItems;
    stats->drawCalls++;
    stats->trisBatched += tris;
	CHECK_GL_ERR
    #endif
}

///////////////////////////////////////////////////////////
// Draw Group
///////////////////////////////////////////////////////////
static void ZR_DrawGroupForward(
    Transform* camera,
    ZRDrawObj* objects,
    i32 numObjects,
    M4x4* projection,
    ZRDrawGroup* group,
    ScreenInfo* scrInfo,
    ZRPerformanceStats* stats)
{
    //ZRGroupId* id = &group->id;
    i32 type = group->data.type;

	// TODO: Support more programs!
	//if (id->program == ZR_SHADER_TYPE_NONE) { return; }
    if (type == ZR_DRAWOBJ_TYPE_NONE) { return; }
	
    // Draw batched meshes
    if (type == ZR_DRAWOBJ_TYPE_MESH)
    {
        #if 0
        ZR_DrawMeshGroupBatched(projection, group, stats);
        #endif
        #if 0
        ZR_DrawMeshGroupTest(
            camera,
            group,
            objects,
            numObjects,
            scrInfo,
            &stats->grouping
        );
        #endif
        //ZR_DrawMeshTest();
        #if 1
        ZR_DrawMeshGroupFallback(
            projection,
            camera,
            group,
            objects,
            numObjects,
            &stats->grouping);
        #endif

        return;
    }

    if (type == ZR_DRAWOBJ_TYPE_TEXT)
    {
        ZR_DrawTextGroup(objects, numObjects, projection, group, scrInfo, stats);
        return;
    }

    printf("No render function for group Id: obj type %d\n", type);
}

#endif // ZRGL_DRAW
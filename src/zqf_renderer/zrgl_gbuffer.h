#ifndef ZRGL_GBUFFER_H
#define ZRGL_GBUFFER_H

#include "zrgl_internal.h"

static void ZRGL_GetQuadHandles(i32* vao, i32* vertCount)
{
    ZRDBMesh* mesh = AssetDb()->GetMeshByName(AssetDb(), "Quad");
	*vao = mesh->handles.vao;
	*vertCount = mesh->data.numVerts;
}

static ZRGBuffer ZRGL_CreateGBuffer(i32 scrWidth, i32 scrHeight)
{
    ZRGBuffer gBuf = {};
    //scrWidth /= 2;
    //scrHeight /= 2;
    #if 1
    printf("Creating gbuffer %d by %d\n", scrWidth, scrHeight);
    glGenFramebuffers(1, &gBuf.fbo);
    CHECK_GL_ERR
    
    glBindFramebuffer(GL_FRAMEBUFFER, gBuf.fbo);
    CHECK_GL_ERR
    ////////////////////////////////////////////////
    // position buffer
    glGenTextures(1, &gBuf.positionTex);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_2D, gBuf.positionTex);
    CHECK_GL_ERR
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scrWidth, scrHeight, 0, GL_RGB, GL_FLOAT, NULL);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    // attach to 0
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuf.positionTex, 0);
    CHECK_GL_ERR
    
    ////////////////////////////////////////////////
    // normals buffer
    glGenTextures(1, &gBuf.normalTex);
    glBindTexture(GL_TEXTURE_2D, gBuf.normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scrWidth, scrHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // attach to 1
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuf.normalTex, 0);

    ////////////////////////////////////////////////
    // colour buffer
    glGenTextures(1, &gBuf.colourTex);
    glBindTexture(GL_TEXTURE_2D, gBuf.colourTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrWidth, scrHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // attach to 2
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuf.colourTex, 0);

    ////////////////////////////////////////////////
    // emission buffer
    glGenTextures(1, &gBuf.emissionTex);
    glBindTexture(GL_TEXTURE_2D, gBuf.emissionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrWidth, scrHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // attach to 3
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gBuf.emissionTex, 0);

    // inform opengl of attachments to FBO
    u32 attachments[4] =
    {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    glDrawBuffers(4, attachments);
    CHECK_GL_ERR

    // then also add render buffer object as depth buffer:    
    // depth map
    glGenRenderbuffers(1, &gBuf.depthRenderBuf);
    CHECK_GL_ERR
    glBindRenderbuffer(GL_RENDERBUFFER, gBuf.depthRenderBuf);
    CHECK_GL_ERR
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scrWidth, scrHeight);
    CHECK_GL_ERR
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gBuf.depthRenderBuf);
    CHECK_GL_ERR

    // Check for completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("gBuffer FBO is not complete\n");
        gBuf.bIsValid = NO;
    }
    else
    {
        gBuf.bIsValid = NO;
        printf("gBuffer created. FBO: %d, res %d/%d\n",
            gBuf.fbo, scrWidth, scrHeight);
    }
    #endif

    // Done. Clear settings
    // printf("gBuffer - binding frame buffer %d\n", gBuf.colourTex);
    // glBindFramebuffer(GL_FRAMEBUFFER, gBuf.colourTex);
    // CHECK_GL_ERR
    // printf("gBuffer built\n");
	return gBuf;
}

static void ZRGL_GeometryPass_Mesh(
	ZRGBuffer* gBuf,
	M4x4* projection,
	M4x4* view,
	ZRDrawObj* objects,
	ZRDrawGroup* group)
{
	ZE_ASSERT(group->data.type == ZR_DRAWOBJ_TYPE_MESH,
			  "Non mesh group passed to mesh draw");
	
	// prog
	GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
	glUseProgram(prog);
	CHECK_GL_ERR

	// setup VAO and textures
	//i32 vao, vertCount;
	//i32 diffuse, emissive;
	ZRMeshDrawHandles h = ZRGL_ExtractDrawHandles(
        AssetDb(), group->data.model.meshIndex, group->data.model.materialIndex);
	
    // Mesh
	glBindVertexArray(h.vao);
	CHECK_GL_ERR
	
	// textures
	ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_colourTex", h.diffuseHandle, g_samplerDataTex2D);
	CHECK_GL_ERR
	ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_emissionTex", h.emissiveHandle, g_samplerDataTex2D);
	CHECK_GL_ERR
	
	ZR_SetProgM4x4(prog, "u_projection", projection->cells);
	CHECK_GL_ERR
	
	M4x4_CREATE(model)
	M4x4_CREATE(modelView)
	
	for (i32 i = 0; i < group->numItems; ++i)
	{
		i32 objIndex = group->indices[i];
		ZRDrawObj *obj = &objects[objIndex];

		ZR_BuildModelMatrix(&model, &obj->t);
		M4x4_SetToIdentity(modelView.cells);
		M4x4_Multiply(modelView.cells, view->cells, modelView.cells);
		M4x4_Multiply(modelView.cells, model.cells, modelView.cells);

		ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
		ZR_SetProgM4x4(prog, "u_model", model.cells);

		glDrawArrays(GL_TRIANGLES, 0, h.vertCount);
		CHECK_GL_ERR
	}
}

static void ZRGL_FillGBuffer(
    ZRGBuffer* gBuf,
    Transform* camera, // Object casting light
	ScreenInfo scrInfo,
	ZRDrawGroup** groups, // objects/geometry to draw
	i32 numGroups,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuf->fbo);
    CHECK_GL_ERR
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
    glUseProgram(prog);
    CHECK_GL_ERR
    glCullFace(GL_BACK);

    M4x4_CREATE(projection)
    M4x4_CREATE(model)
    M4x4_CREATE(view)
    M4x4_CREATE(modelView)

    COM_SetupDefault3DProjection(projection.cells, scrInfo.aspectRatio);
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    CHECK_GL_ERR

    ZR_BuildViewMatrix(&view, camera);
    
    for (i32 i = 0; i < numGroups; ++i)
    {
        ZRDrawGroup* group = groups[i];
		switch (group->data.type)
        {
            case ZR_DRAWOBJ_TYPE_MESH:
            ZRGL_GeometryPass_Mesh(gBuf, &projection, &view, objects, group);
            break;
            default:
            #ifdef ZR_REPORT_GROUP_ERRORS
			printf("FillGBuffer - Unknown draw group type %d\n",
				group->data.type);
            #endif
			break;
        }
    }
    
    // clear settings
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * Draw debug GBuffer Quad to screen
 */
static void ZRGL_DrawDebugGBufferCombine(ZRGBuffer* gBuf)
{
    // disable depth stuff
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    GLint prog = g_programs[ZR_SHADER_TYPE_COMBINE_GBUFFER].handle;
    glUseProgram(prog);

    M4x4_CREATE(projection)
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    M4x4_CREATE(modelView)
    // gbuffer quad is drawn in screen space, -1 to 1 so scale up:
    M4x4_SetScale(modelView.cells, 2, 2, 2);
    //M4x4_SetScale(modelView.cells, 1, 1, 1);
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);

    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_colourTex", gBuf->colourTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_normalTex", gBuf->normalTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE2, 2, "u_positionTex", gBuf->positionTex, g_samplerDataTex2D);
    
    i32 vao;
	i32 vertCount;
	ZRGL_GetQuadHandles(&vao, &vertCount);
	if (vao == 0)
    {
        printf("GBuffer combine - vao %d is invalid!\n", vao);
    }
    else
    {
        glBindVertexArray(vao);
        CHECK_GL_ERR
        glBindTexture(GL_TEXTURE_2D, gBuf->colourTex);
        CHECK_GL_ERR

        glDrawArrays(GL_TRIANGLES, 0, vertCount);
        CHECK_GL_ERR
    }
	
    // clean up
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);

}

/**
 * Draw GBuffer quad with a directional light
 */
static void ZRGL_GBufferDrawDirectLight(
    ZRGBuffer* gBuf, Vec3 lightWorldPos, Vec3 lightWorldDir, Vec3 lightColour,
    f32 lightMultiplier, f32 lightRange)
{
    GLint prog = g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_DIRECT].handle;
    glUseProgram(prog);

    M4x4_CREATE(projection)
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    M4x4_CREATE(modelView)
    // gbuffer quad is drawn in screen space, -1 to 1 so scale up:
    M4x4_SetScale(modelView.cells, 2, 2, 2);
    //M4x4_SetScale(modelView.cells, 1, 1, 1);
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);

    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_positionTex", gBuf->positionTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_normalTex", gBuf->normalTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE2, 2, "u_colourTex", gBuf->colourTex, g_samplerDataTex2D);
    
    //printf("GBuf light at %.3f, %.3f, %.3f - range %.3f\n",
    //    lightWorldPos.x, lightWorldPos.y, lightWorldPos.z, 10.f
    //);
    ZR_SetProgVec3f(prog, "u_lightWorldPos", lightWorldPos);
    ZR_SetProgVec3f(prog, "u_lightWorldDir", lightWorldDir);
    ZR_SetProgVec3f(prog, "u_lightColour", lightColour);
    
    ZR_SetProg1f(prog, "u_lightMultiplier", lightMultiplier);
    ZR_SetProg1f(prog, "u_lightRange", lightRange);

    i32 vao, vertCount;
	ZRGL_GetQuadHandles(&vao, &vertCount);
	glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, gBuf->colourTex);

    glDrawArrays(GL_TRIANGLES, 0, vertCount);

   
}

/**
 * Draw GBuffer quad with a point light
 */
static void ZRGL_GBufferDrawPointLight(
    ZRGBuffer* gBuf, Vec3 lightWorldPos, Vec3 lightWorldDir, Vec3 lightColour,
    f32 lightMultiplier, f32 lightRange)
{
    GLint prog = g_programs[ZR_SHADER_TYPE_GBUFFER_LIGHT_POINT].handle;
    glUseProgram(prog);

    M4x4_CREATE(projection)
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    M4x4_CREATE(modelView)
    // gbuffer quad is drawn in screen space, -1 to 1 so scale up:
    M4x4_SetScale(modelView.cells, 2, 2, 2);
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);

    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_positionTex", gBuf->positionTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_normalTex", gBuf->normalTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE2, 2, "u_colourTex", gBuf->colourTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE3, 3, "u_emissionTex", gBuf->emissionTex, g_samplerDataTex2D);
    
    ZR_SetProgVec3f(prog, "u_lightWorldPos", lightWorldPos);
    ZR_SetProgVec3f(prog, "u_lightWorldDir", lightWorldDir);
    ZR_SetProgVec3f(prog, "u_lightColour", lightColour);
    
    ZR_SetProg1f(prog, "u_lightMultiplier", lightMultiplier);
    ZR_SetProg1f(prog, "u_lightRange", lightRange);

    i32 vao, vertCount;
	ZRGL_GetQuadHandles(&vao, &vertCount);
	glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, gBuf->colourTex);

    glDrawArrays(GL_TRIANGLES, 0, vertCount);

   
}

static void ZRGL_DrawEmission(f32 aspectRatio)
{
    Vec2 pos = {};
    Vec2 size = { 2, 2 };
    Vec2 uvMin = { 0, 0 };
    Vec2 uvMax = { 1, 1 };
    i32 texHandle = g_gBuffer.emissionTex;
    ZRGL_DrawDebugQuad(pos, size, uvMin, uvMax, texHandle, aspectRatio);
}

/**
 * Draw gbuffer components for debug
 */
static void ZRGL_DrawGBufferDebugQuads(f32 aspectRatio)
{
    f32 debugQuadSize = 0.5f;
    f32 debugQuadPosOuter = 0.75f;
    f32 debugQuadPosInner = 0.25f;
    #if 1 // Draw render texture debug A
    i32 texA = g_gBuffer.positionTex;
    i32 texB = g_gBuffer.normalTex;
    i32 texC = g_gBuffer.colourTex;
    i32 texD = g_gBuffer.emissionTex;
    //i32 texC = g_rendToTexFB.colourTex;
    //i32 texD = g_shadowMapFB.depthTex;
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

#endif // ZRGL_GBUFFER_H
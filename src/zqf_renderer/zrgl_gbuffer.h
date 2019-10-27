#ifndef ZRGL_GBUFFER_H
#define ZRGL_GBUFFER_H

#include "zrgl_internal.h"

static ZRGBuffer ZRGL_CreateGBuffer(i32 scrWidth, i32 scrHeight)
{
    ZRGBuffer gBuf = {};
    #if 1
    printf("Creating gbuffer %d by %d\n", scrWidth, scrHeight);
    glGenFramebuffers(1, &gBuf.fbo);
    CHECK_GL_ERR
    
    glBindFramebuffer(GL_FRAMEBUFFER, gBuf.fbo);
    CHECK_GL_ERR
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
    
    // normals buffer
    glGenTextures(1, &gBuf.normalTex);
    glBindTexture(GL_TEXTURE_2D, gBuf.normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scrWidth, scrHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // attach to 1
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuf.normalTex, 0);

    // colour buffer
    glGenTextures(1, &gBuf.colourTex);
    glBindTexture(GL_TEXTURE_2D, gBuf.colourTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrWidth, scrHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // attach to 2
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuf.colourTex, 0);

    // inform opengl of attachments to FBO
    u32 attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

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
    glBindFramebuffer(GL_FRAMEBUFFER, gBuf.colourTex);
    printf("gBuffer built\n");
	return gBuf;
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
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);

    GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
    glUseProgram(prog);
    glCullFace(GL_BACK);

    M4x4_CREATE(projection)
    M4x4_CREATE(model)
    M4x4_CREATE(view)
    M4x4_CREATE(modelView)

    COM_SetupDefault3DProjection(projection.cells, scrInfo.aspectRatio);
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);

    ZR_BuildViewMatrix(&view, camera);
    
    for (i32 i = 0; i < numGroups; ++i)
    {
        ZRDrawGroup* group = groups[i];
        if (group->id.objType != ZR_DRAWOBJ_TYPE_MODEL) { continue; }
        ZRPrefab* prefab = ZRGL_GetPrefab(group->id.prefab);
        glBindVertexArray(prefab->geometry.vao);
        glBindTexture(GL_TEXTURE_2D, prefab->textures.diffuse);
        for (i32 j = 0; j < group->numItems; ++j)
        {
            i32 objIndex = group->indices[j];
            ZRDrawObj* obj = &objects[objIndex];
            ZE_ASSERT(obj->type == ZR_DRAWOBJ_TYPE_MODEL,
                "GBuffer fill by non model obj");
            ZR_BuildModelMatrix(&model, &obj->t);
            M4x4_SetToIdentity(modelView.cells);
            M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
            M4x4_Multiply(modelView.cells, model.cells, modelView.cells);

            ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
            ZR_SetProgM4x4(prog, "u_model", model.cells);

            glDrawArrays(GL_TRIANGLES, 0, prefab->geometry.vertexCount);
            stats->drawCallsGBuffer++;
        }
    }
    
    // clear settings
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void ZRGL_CombineGBuffer(ZRGBuffer* gBuf)
{
    GLint prog = g_programs[ZR_SHADER_TYPE_COMBINE_GBUFFER].handle;
    glUseProgram(prog);

    M4x4_CREATE(projection)
    ZR_SetProgM4x4(prog, "u_projection", projection.cells);
    M4x4_CREATE(modelView)
    ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
    M4x4_SetScale(modelView.cells, 2, 2, 2);

    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_colourTex", gBuf->colourTex, g_samplerDataTex2D);
    
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_normalTex", gBuf->normalTex, g_samplerDataTex2D);
    ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE2, 2, "u_positionTex", gBuf->positionTex, g_samplerDataTex2D);
    

    ZRPrefab* prefab = &g_prefabs[ZR_PREFAB_TYPE_QUAD];
	glBindVertexArray(prefab->geometry.vao);
    glBindTexture(GL_TEXTURE_2D, gBuf->colourTex);

    glDrawArrays(GL_TRIANGLES, 0, prefab->geometry.vertexCount);
}

#endif // ZRGL_GBUFFER_H
#ifndef ZRGL_SHADOWS_H
#define ZRGL_SHADOWS_H

#include "zrgl_internal.h"

static void ZR_DrawMeshGroupShadowMap(
    ZRShadowCaster* shadow,
    ZRDrawGroup* group,
    i32 vaoHandle,
    i32 vertexCount,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats)
{
    #if 1
    GLint prog = g_programs[ZR_SHADER_TYPE_SHADOW_MAP].handle;
    glUseProgram(prog);
    CHECK_GL_ERR
    ZR_SetProgM4x4(prog, "u_projection", shadow->projection.cells);
    M4x4_CREATE(model)
    M4x4_CREATE(modelView)
    // Prepare geometry
    glBindVertexArray(vaoHandle);
	CHECK_GL_ERR
    for (i32 i = 0; i < group->numItems; ++i)
    {
        i32 objIndex = group->indices[i];
        ZRDrawObj* obj = &objects[objIndex];
        M4x4_SetToIdentity(modelView.cells);
        ZR_BuildModelMatrix(&model, &obj->t);
        M4x4_Multiply(modelView.cells, shadow->view.cells, modelView.cells);
        M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
        
        ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        CHECK_GL_ERR
        stats->drawCallsShadows++;
    }
	#endif
}

//#define ZRGL_DRAW_SHADOWS_TO_BACK_FACES

/**
 * Draw front faces:
 * bias = 0.005;
 */
static void ZRGL_WriteTestShadowMap(
    Transform* camera, // Object casting light
	ScreenInfo scrInfo,
	ZRDrawGroup** groups, // objects/geometry to draw
	i32 numGroups,
    ZRDrawObj* objects,
    i32 numObjects,
    ZRGroupingStats* stats)
{
    glClearColor(0, 0, 0, 1);
    ZRFrameBuffer* fb = &g_shadowMapFB; // setup to write to depth map frame buffer
    stats->shadowMaps++;
	glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
    CHECK_GL_ERR

    glViewport(0, 0, ZR_SHADOW_MAP_WIDTH, ZR_SHADOW_MAP_HEIGHT);
    CHECK_GL_ERR
	glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERR
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERR
    
    // Draw to back faces
    #ifdef ZRGL_DRAW_SHADOWS_TO_BACK_FACES
    glCullFace(GL_FRONT);
    #endif

    // Setup shadow transforms
    ZRShadowCaster* shadow = &g_shadow;
    shadow->worldT = *camera;
	COM_SetupOrthoProjection(shadow->projection.cells, 40, 1);
    //COM_SetupDefault3DProjection(shadow->projection.cells, 1);
    ZR_BuildViewMatrix(&shadow->view, camera);

	// Draw
    for (i32 i = 0; i < numGroups; ++i)
    {
        ZRDrawGroup* group = groups[i];
        switch (group->data.type)
        {
            case ZR_DRAWOBJ_TYPE_MESH:
            {
				// TODO - reimplement...?
				ILLEGAL_CODE_PATH
				i32 vao = 0;
				i32 vertCount = 0;
                ZR_DrawMeshGroupShadowMap(
                    shadow,
                    group,
                    vao,
                    vertCount,
                    objects,
                    numObjects,
                    stats
                );
            } break;
        }
    }
    
	// Restore previous settings
    
    // Draw to back faces
    #ifdef ZRGL_DRAW_SHADOWS_TO_BACK_FACES
    glCullFace(GL_BACK);
    #endif

	glViewport(0, 0, scrInfo.width, scrInfo.height);
    CHECK_GL_ERR
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR
    ZRGL_ClearColourDefault();
}

#endif // ZRGL_SHADOWS_H
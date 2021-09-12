#include "../ze_opengl_internal.h"

ze_external void ZRSandbox_DrawSpriteBatch()
{
    //////////////////////////////////////////////////
    // constants
    //////////////////////////////////////////////////
    const float quadVerts[18] =
    {
        -0.5, -0.5, 0, // bottom left
        0.5, -0.5, 0, // bottom right
        0.5, 0.5, 0, // top right

        -0.5, -0.5, 0, // bottom left
        0.5, 0.5, 0, // top right
        -0.5, 0.5, 0 // top left
    };

    //////////////////////////////////////////////////
    // persist vars
    //////////////////////////////////////////////////
    local_persist i32 g_bInitialised = NO;
    local_persist ZRShader g_shader = {};
    local_persist i32 g_meshId;

    //////////////////////////////////////////////////
    // local vars
    //////////////////////////////////////////////////
    zErrorCode err;

    //////////////////////////////////////////////////
    // init
    //////////////////////////////////////////////////
    if (g_bInitialised == NO)
    {
        g_bInitialised = YES;
        err = ZRGL_CreateProgram(
            draw_sprite_batch_vert_text,
            draw_sprite_batch_frag_text,
            "sprite_batch_test",
            ZR_DRAWOBJ_TYPE_QUAD,
            YES,
            &g_shader);
        ZE_ASSERT(err == ZE_ERROR_NONE, "create prog failed")

        g_meshId = ZAssets_GetMeshByName(ZE_EMBEDDED_QUAD_NAME)->header.id;
    }

    //////////////////////////////////////////////////
    // execute
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    // draw

    // acquire mesh handles
    ZRMeshHandles* mesh = ZRGL_GetMeshHandles(g_meshId);
    
    // clear
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // not interested in depth buffer for this
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    i32 instanceCount = 4;

    // bind gpu objects
    glBindVertexArray(mesh->vao);
    glUseProgram(g_shader.handle);

    M4x4_CREATE(projection)
    M4x4_CREATE(model)

    ZR_SetProg1i(g_shader.handle, "u_instanceCount", instanceCount);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", projection.cells);
    // ZR_SetProgM4x4(g_shader.handle, "u_modelView", model.cells);

    // draw
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexCount, instanceCount);
}

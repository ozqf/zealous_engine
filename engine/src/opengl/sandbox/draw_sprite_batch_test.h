#include "../ze_opengl_internal.h"

ze_external void ZRSandbox_DrawSpriteBatch()
{
    //////////////////////////////////////////////////
    // persist vars
    //////////////////////////////////////////////////
    local_persist i32 g_bInitialised = NO;
    local_persist ZRShader g_shader = {};
    local_persist i32 g_meshId;
    local_persist i32 g_diffuseTexHandle;

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

        // create a little sprite sheet
        ZRTexture* tex = ZAssets_AllocTex(32, 32, "sprite_batch_test_atlas");
        ZGen_FillTexture(tex, COLOUR_U32_BLACK);
        ZGen_FillTextureRect(tex, COLOUR_U32_WHITE, { 16, 16 }, { 16, 16 });
        g_diffuseTexHandle = ZRGL_GetTextureHandle(tex->header.id);
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
    
    ZR_PrepareTextureUnit2D(
        g_shader.handle, GL_TEXTURE0, 0, "u_diffuseTex", g_diffuseTexHandle, 0);

    // draw
    glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexCount, instanceCount);
}

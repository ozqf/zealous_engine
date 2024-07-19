#include "../ze_opengl_internal.h"

internal i32 g_initialised = NO;
internal ZRShader g_shader = {};

ze_external void ZRGL_DrawMesh(
    ZRDrawCmdMesh* meshCmd,  M4x4* view, M4x4* projection)
{
    if (!g_initialised)
    {
        g_initialised = true;
        ZE_SetFatalError(Platform_Fatal);
        zErrorCode err = ZRGL_CreateProgram(draw_single_mesh_vert_text, draw_single_mesh_frag_text, "draw_mesh", &g_shader);
        if (err != ZE_ERROR_NONE)
        {
            Platform_Fatal("Shader compile failed");
        }
    }

    glUseProgram(g_shader.handle);
    CHECK_GL_ERR

    ZRDrawObj* obj = &meshCmd->obj;
    Vec4 albedoTint = ColourF32ToVec4(obj->data.model.albedoColour);
    

    // retrieve assets
    ZRMeshAsset *mesh = ZAssets_GetMeshById(obj->data.model.meshId);

    ZE_ASSERT(mesh != NULL, "Mesh is null");

    // material and textures
    ZRMaterial *mat = ZAssets_GetMaterialById(obj->data.model.materialId);
    ZRMeshHandles* meshHandles = ZRGL_GetMeshHandles(mesh->header.id);
    ZE_ASSERT(meshHandles != NULL, "Draw mesh got no mesh handles")
    u32 vao = meshHandles->vao;
    u32 numVerts = meshHandles->vertexCount;

    u32 diffuseHandle = ZRGL_GetTextureHandle(mat->diffuseTexId);

    ZR_PrepareTextureUnit2D(
        g_shader.handle,
        GL_TEXTURE0, 0, "u_diffuseTex", diffuseHandle, 0);
    CHECK_GL_ERR

    M4x4_CREATE(modelView)
    #if 1
    M4x4_CREATE(model)
    Transform_ToM4x4(&obj->t, &model);
    M4x4_Multiply(modelView.cells, view->cells, modelView.cells);
    M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
    #endif

    ZR_SetProgM4x4(g_shader.handle, "u_modelView", modelView.cells);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", projection->cells);
    ZR_SetProgVec4f(g_shader.handle, "u_colour", albedoTint);

    glBindVertexArray(vao);
    CHECK_GL_ERR
    glDrawArrays(GL_TRIANGLES, 0, numVerts);
    CHECK_GL_ERR
}

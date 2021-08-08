#include "ze_opengl_internal.h"

internal i32 g_initialised = NO;
internal ZRShader g_shader = {};

ze_external void ZRGL_DrawMesh(
    ZRDrawCmdMesh* meshCmd,  M4x4* view, M4x4* projection)
{
    if (!g_initialised)
    {
        g_initialised = true;
        ZE_SetFatalError(Platform_Fatal);
        zErrorCode err = ZRGL_CreateProgram(draw_single_mesh_vert_text, draw_single_mesh_frag_text, "draw_mesh", ZR_DRAWOBJ_TYPE_MESH, NO, &g_shader);
        if (err != ZE_ERROR_NONE)
        {
            Platform_Fatal("Shader compile failed");
        }
    }

    glUseProgram(g_shader.handle);
    CHECK_GL_ERR

    ZRDrawObj* obj = &meshCmd->obj;

    // retrieve assets
    ZRMeshAsset *mesh = ZAssets_GetMeshById(obj->data.model.meshId);
    ZE_ASSERT(mesh != NULL, "Mesh is null");

    // material and textures
    ZRMaterial *mat = ZAssets_GetMaterialById(obj->data.model.materialId);
    // ZRTexture* diffuse = ZAssets_GetTexById(mat->diffuseTexId);
    // ZRTexture* diffuse = ZAssets_GetTexByName(FALLBACK_TEXTURE_NAME);
    // ZE_ASSERT(diffuse != NULL, "Diffuse is null")

    // ZRGLHandles* meshHandle = ZRGL_GetHandleData(mesh->header.id);
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
    // M4x4_SetToIdentity(projection->cells);
    // M4x4_SetToIdentity(modelMatrix.cells);
    // M4x4_SetToIdentity(viewMatrix.cells);
    #if 0
    M4x4_SetToIdentity(modelView.cells);
    M4x4_RotateByAxis(modelView.cells, 45.f * DEG2RAD, 1, 0, 0);
    M4x4_RotateByAxis(modelView.cells, 45.f * DEG2RAD, 0, 1, 0);
    modelView.posZ = -2.f;
    M4x4_SetToIdentity(projection->cells);

    ZE_SetupDefault3DProjection(projection->cells, 16.f / 9.f);
    #endif
    #if 1
    M4x4_CREATE(model)
    Transform_ToM4x4(&obj->t, &model);
    M4x4_Multiply(modelView.cells, view->cells, modelView.cells);
    M4x4_Multiply(modelView.cells, model.cells, modelView.cells);
    #endif

    ZR_SetProgM4x4(g_shader.handle, "u_modelView", modelView.cells);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", projection->cells);

    // printf("ZRGL draw mesh id %d. vao %d, verts %d at worldPos %.3f, %.3f, %.3f\n",
    //     mesh->header.id, vao, numVerts, model.posX, model.posY, model.posZ);
    // printf("ZRGL draw mesh id %d. vao %d, verts %d at modelView %.3f, %.3f, %.3f\n",
    //        mesh->header.id, vao, numVerts, modelView.posX, modelView.posY, modelView.posZ);

    glBindVertexArray(vao);
    CHECK_GL_ERR
    glDrawArrays(GL_TRIANGLES, 0, numVerts);
    CHECK_GL_ERR
}

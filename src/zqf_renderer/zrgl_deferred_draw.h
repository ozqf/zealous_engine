#ifndef ZRGL_DEFERRED_DRAW_H
#define ZRGL_DEFERRED_DRAW_H

#include "zrgl_internal.h"

static void ZRGL_GeometryPass_Mesh(
	ZRGBuffer* gBuf,
	M4x4* projection,
	M4x4* view,
	ZRDrawObj* objects,
	ZRDrawGroup* group)
{
	ZE_ASSERT(group->data.type == ZR_DRAWOBJ_TYPE_MESH,
		"Non mesh group passed to mesh draw");
	
	M4x4_CREATE(model)
	M4x4_CREATE(modelView)
	
	// setup VAO and textures
	ZRMeshHandles mesh;
	ZRDB_GetMeshHandlesByIndex(group->data.model.meshIndex, &mesh);
	i32 vao = mesh.vao;
	i32 vertCount = mesh.vertexCount;
	glBindVertexArray(vao);
	
	ZRMaterial mat;
	ZRDB_GetMaterialByIndex(group->data.model.materialIndex, &mat);
	
	GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
    glUseProgram(prog);
	
	for (i32 i = 0; i < group->numItems; ++i)
	{
		i32 objIndex = group->indices[i];
		ZRDrawObj* obj = &objects[objIndex];
		
		ZR_BuildModelMatrix(&model, &obj->t);
        M4x4_SetToIdentity(modelView.cells);
        M4x4_Multiply(modelView.cells, view->cells, modelView.cells);
        M4x4_Multiply(modelView.cells, model.cells, modelView.cells);

        ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
        ZR_SetProgM4x4(prog, "u_model", model.cells);

        glDrawArrays(GL_TRIANGLES, 0, vertCount);
	}
}

#endif // ZRGL_DEFERRED_DRAW_H
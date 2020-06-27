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

#endif // ZRGL_DEFERRED_DRAW_H
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

	// setup VAO and textures
	i32 vao, vertCount;
	i32 diffuse, emissive;

	/////////////////////////////////////////////////////////////
    // Mesh
	ZRDBMesh* mesh = AssetDb()->GetMeshByIndex(AssetDb(), group->data.model.meshIndex);
	vao = mesh->handles.vao;
	vertCount = mesh->data.numVerts;
	glBindVertexArray(vao);
	CHECK_GL_ERR
	
	GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
	glUseProgram(prog);
	CHECK_GL_ERR

	/////////////////////////////////////////////////////////////
	// pull in material info
	ZRMaterial* mat = AssetDb()->GetMaterialByIndex(AssetDb(), group->data.model.materialIndex);
	diffuse = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->diffuseTexIndex);
	emissive = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->emissionTexIndex);
	
	if (g_verboseFrame == YES)
	{
		printf("Geometry pass - %d items. mesh %s (%d verts) mat %s\n",
			group->numItems, mesh->header.fileName, mesh->data.numVerts, mat->name);
		printf("\tHandles vao %d, diffuse %d emissive %d\n",
			vao, diffuse, emissive);
	}
	
	ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_colourTex", diffuse, g_samplerDataTex2D);
	CHECK_GL_ERR
	ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_emissionTex", emissive, g_samplerDataTex2D);
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

		glDrawArrays(GL_TRIANGLES, 0, vertCount);
		CHECK_GL_ERR
	}
}

#endif // ZRGL_DEFERRED_DRAW_H
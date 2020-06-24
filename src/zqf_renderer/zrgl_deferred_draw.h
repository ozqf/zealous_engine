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
	#if 0
	ZRMeshHandles mesh;
	ZRDB_GetMeshHandlesByIndex(group->data.model.meshIndex, &mesh);
	vao = mesh.vao;
	vertCount = mesh.vertexCount;
	glBindVertexArray(vao);
	#endif
	#if 1
	ZRMeshHandles mesh;
	//ZRDB_GetMeshHandlesByName("Cube", &mesh);
	AssetDb()->GetMeshHandleByName(AssetDb(), "Cube", &mesh);
	vao = mesh.vao;
	vertCount = mesh.vertexCount;
	glBindVertexArray(vao);
	CHECK_GL_ERR
	#endif
	
	GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
	glUseProgram(prog);
	CHECK_GL_ERR

	i32 diffuse, emissive;
	#if 1 // pull in material info
	ZRMaterial* mat = AssetDb()->GetMaterialByIndex(AssetDb(), group->data.model.materialIndex);
	diffuse = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->diffuseTexIndex);
	emissive = AssetDb()->GetTextureHandleByIndex(AssetDb(), mat->emissionTexIndex);
	#endif
	#if 0 // get texture handles directly
	ZRDBTexture* tex;
	tex = AssetDb()->GetTextureByName(AssetDb(), ZQF_R_DEFAULT_DIFFUSE_TEX);
	diffuse = tex->apiHandle;
	tex = AssetDb()->GetTextureByName(AssetDb(), "data/debug_black.png");
	emissive = tex->apiHandle;
	#endif

	#if 0
	printf("Geometry pass mat %s handles: %d, %d\n",
		mat->name, diffuse, emissive);
	#endif
	
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
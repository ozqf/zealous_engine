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
	ZRDB_GetMeshHandlesByName("Cube", &mesh);
	vao = mesh.vao;
	vertCount = mesh.vertexCount;
	glBindVertexArray(vao);
	#endif
	
	GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
	glUseProgram(prog);

	//ZRMaterial mat;
	//ZRDB_GetMaterialByIndex(group->data.model.materialIndex, &mat);
	i32 diffuse = ZRDB_GetTexHandleByName(ZQF_R_DEFAULT_DIFFUSE_TEX);
	//i32 diffuse = ZRDB_GetTexHandleByName("data/WALL03_7.png");
	//i32 emissive = ZRDB_GetTexHandleByName("data/debug_white.png");
	i32 emissive = ZRDB_GetTexHandleByName("data/debug_black.png");
	//mat.diffuseTexHandle = 1;
	//mat.emissionTexHandle = 1;
	//printf("ZR Geom pass mesh %d diffuse %d emissive %d\n", vao, diffuse, emissive);
	ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE0, 0, "u_colourTex", diffuse, g_samplerDataTex2D);
	ZR_PrepareTextureUnit2D(
        prog, GL_TEXTURE1, 1, "u_emissionTex", emissive, g_samplerDataTex2D);
	
	ZR_SetProgM4x4(prog, "u_projection", projection->cells);
	
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
	}
}

static void ZRGL_GeometryPass_Prefab(
	ZRGBuffer *gBuf,
	M4x4 *projection,
	M4x4 *view,
	ZRDrawObj *objects,
	ZRDrawGroup *group)
{
	GLint prog = g_programs[ZR_SHADER_TYPE_BUILD_GBUFFER].handle;
    glUseProgram(prog);

	ZRPrefab *prefab = ZRGL_GetPrefab(group->data.prefab.prefabId);
	glBindVertexArray(prefab->geometry.vao);

	// Prepare textures
	ZR_PrepareTextureUnit2D(
		prog, GL_TEXTURE0, 0, "u_colourTex", prefab->textures.diffuse, g_samplerDataTex2D);

	char *emissionTexName = "data/debug_white.png";
	#if 1
	if (group->data.prefab.prefabId == ZR_PREFAB_TYPE_DEBUG_PLAYER_PROJECTILE)
	{
		emissionTexName = "data/debug_white.png";
	}
	else
	{
		emissionTexName = "data/debug_black.png";
	}
	#endif

	i32 emissionTexIndex = ZRDB_GetTexIndexByName(emissionTexName);
	i32 emissionTexHandle = ZRDB_GetTexHandleByIndex(emissionTexIndex);
	
	M4x4_CREATE(model)
	M4x4_CREATE(modelView)
	
	//printf("Binding tex handle %d to tex unit 1\n", emissionTexHandle);
	ZR_PrepareTextureUnit2D(
		prog, GL_TEXTURE1, 1, "u_emissionTex", emissionTexHandle, g_samplerB);

	for (i32 j = 0; j < group->numItems; ++j)
	{
		i32 objIndex = group->indices[j];
		ZRDrawObj *obj = &objects[objIndex];
		ZE_ASSERT(obj->data.type == ZR_DRAWOBJ_TYPE_PREFAB,
				  "GBuffer fill by non model obj");
		ZR_BuildModelMatrix(&model, &obj->t);
		M4x4_SetToIdentity(modelView.cells);
		M4x4_Multiply(modelView.cells, view->cells, modelView.cells);
		M4x4_Multiply(modelView.cells, model.cells, modelView.cells);

		ZR_SetProgM4x4(prog, "u_modelView", modelView.cells);
		ZR_SetProgM4x4(prog, "u_model", model.cells);

		glDrawArrays(GL_TRIANGLES, 0, prefab->geometry.vertexCount);
		//stats->drawCallsGBuffer++;
	}
}

#endif // ZRGL_DEFERRED_DRAW_H
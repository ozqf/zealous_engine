/*
Loads embedded primitives and assets into the asset db.
*/
#include "../../internal_headers/zengine_internal.h"

#include "bw_charset.h"
#include "primitive_cube.h"
#include "primitive_quad.h"

ze_external zErrorCode ZEmbedded_Init()
{
	////////////////////////////////////////////////
	// Create embedded textures
	////////////////////////////////////////////////
	ZRTexture *tex;
	// magenta fallback
	tex = ZAssets_AllocTex(64, 64, FALLBACK_TEXTURE_NAME);
	ZGen_FillTexture(tex, { 255, 0, 255, 255 });

	tex = ZAssets_AllocTex(64, 64, FALLBACK_TEXTURE_WHITE);
	ZGen_FillTexture(tex, COLOUR_U32_WHITE);

	ZRMaterial* mat;
	mat = ZAssets_BuildMaterial(FALLBACK_MATERIAL_NAME, FALLBACK_TEXTURE_NAME, NULL);

	tex = ZAssets_AllocTex(64, 64, "chequer_texture");
	ZGen_FillTexture(tex, { 50, 50, 50, 255 });
	ZGen_FillTextureRect(tex, {100, 100, 100, 255}, {0, 0}, {32, 32});
	ZGen_FillTextureRect(tex, {100, 100, 100, 255}, {32, 32}, {32, 32});

	tex = ZAssets_AllocTex(32, 32, "white_texture");
	ZGen_FillTexture(tex, COLOUR_U32_WHITE);

	// embedded black and white charset
	tex = ZAssets_AllocTex(bw_charset_width, bw_charset_height, FALLBACK_CHARSET_TEXTURE_NAME);
	TexGen_DecodeBW(
		bw_charset_bytes,
		bw_charset_num_bytes,
		tex->data,
		bw_charset_width,
		bw_charset_height,
		COLOUR_U32_CYAN, COLOUR_U32_BLACK);

	// embedded black and white charset
	tex = ZAssets_AllocTex(
		bw_charset_width, bw_charset_height, FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME);
	TexGen_DecodeBW(
		bw_charset_bytes,
		bw_charset_num_bytes,
		tex->data,
		bw_charset_width,
		bw_charset_height,
		COLOUR_U32_WHITE, COLOUR_U32_EMPTY);
	// printf("Assigned %s to %d\n", FALLBACK_CHARSET_TEXTURE_NAME, tex->header.id);

	////////////////////////////////////////////////
	// Create embedded materials
	////////////////////////////////////////////////

	ZAssets_BuildMaterial(FALLBACK_CHEQUER_MATERIAL, "chequer_texture", NULL);
	ZAssets_BuildMaterial("white", "white_texture", NULL);

	////////////////////////////////////////////////
	// Created embedded meshes
	////////////////////////////////////////////////
	ZRMeshData mesh;
	ZRMeshAsset *asset;
	mesh = g_meshCube;
	printf("Create placeholder %s (%d/%d verts)\n",
		ZE_EMBEDDED_CUBE_NAME, g_meshCube.numVerts, g_meshCube.maxVerts);
	asset = ZAssets_AllocEmptyMesh(ZE_EMBEDDED_CUBE_NAME, mesh.numVerts);
	asset->data.CopyData(mesh);
	
	mesh = g_meshPrimitive_quad;
	asset = ZAssets_AllocEmptyMesh(ZE_EMBEDDED_QUAD_NAME, mesh.numVerts);
	asset->data.CopyData(mesh);

	asset = ZAssets_AllocEmptyMesh(ZE_EMBEDDED_SCREEN_QUAD_NAME, mesh.numVerts);
	asset->data.AddVert({ -1, -1, 0 }, { 0, 0 }, { 0, 0, -1 });
	asset->data.AddVert({ 1, -1, 0 }, { 1, 0 }, { 0, 0, -1 });
	asset->data.AddVert({ 1, 1, 0 }, { 1, 1 }, { 0, 0, -1 });

	asset->data.AddVert({ -1, -1, 0 }, { 0, 0 }, { 0, 0, -1 });
	asset->data.AddVert({ 1, 1, 0 }, { 1, 1 }, { 0, 0, -1 });
	asset->data.AddVert({ -1, 1, 0 }, { 0, 1 }, { 0, 0, -1 });

	return ZE_ERROR_NONE;
}

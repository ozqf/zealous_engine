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

	ZRMaterial* mat;
	mat = ZAssets_BuildMaterial(FALLBACK_MATERIAL_NAME, FALLBACK_TEXTURE_NAME, NULL);

	tex = ZAssets_AllocTex(64, 64, "chequer_texture");
	ZGen_FillTexture(tex, { 50, 50, 50, 255 });
	ZGen_FillTextureRect(tex, {100, 100, 100, 255}, {0, 0}, {32, 32});
	ZGen_FillTextureRect(tex, {100, 100, 100, 255}, {32, 32}, {32, 32});

	ZAssets_BuildMaterial(FALLBACK_CHEQUER_MATERIAL, "chequer_texture", NULL);

	// embedded black and white charset
	tex = ZAssets_AllocTex(bw_charset_width, bw_charset_height, FALLBACK_CHARSET_TEXTURE_NAME);
	TexGen_DecodeBW(
		bw_charset_bytes,
		bw_charset_num_bytes,
		tex->data,
		bw_charset_width,
		bw_charset_height,
		COLOUR_U32_GREEN, COLOUR_U32_BLACK);

	// embedded black and white charset
	tex = ZAssets_AllocTex(
		bw_charset_width, bw_charset_height, FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME);
	TexGen_DecodeBW(
		bw_charset_bytes,
		bw_charset_num_bytes,
		tex->data,
		bw_charset_width,
		bw_charset_height,
		COLOUR_U32_GREEN, COLOUR_U32_EMPTY);
	// printf("Assigned %s to %d\n", FALLBACK_CHARSET_TEXTURE_NAME, tex->header.id);

	////////////////////////////////////////////////
	// Create embedded materials
	////////////////////////////////////////////////

	////////////////////////////////////////////////
	// Created embedded meshes
	////////////////////////////////////////////////
	ZRMeshData mesh;
	mesh = g_meshCube;
	printf("Create placeholder %s (%d/%d verts)\n",
		ZE_EMBEDDED_CUBE_NAME, g_meshCube.numVerts, g_meshCube.maxVerts);
	ZRMeshAsset *asset = ZAssets_AllocEmptyMesh(ZE_EMBEDDED_CUBE_NAME, mesh.numVerts);
	asset->data.CopyData(mesh);
	// printf("Copied:\n");
	// g_meshCube.PrintVerts();
	// printf("to:\n");
	// asset->data.PrintVerts();
	return ZE_ERROR_NONE;
}

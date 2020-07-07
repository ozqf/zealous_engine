/**
 * Some ultra-basic box-modelling functions
 * for procedural mesh generation.
 */
#include "zr_embedded.h"

// static i32 MeshGen_QuadSize()
// {
// 	// uncompressed quads, so 6 verts.
// 	return
// 			(sizeof(Vec3) * MESH_GEN_VERTS_PER_QUAD)
// 		+	(sizeof(Vec2) * MESH_GEN_VERTS_PER_QUAD)
// 		+	(sizeof(Vec3) * MESH_GEN_VERTS_PER_QUAD);
// }

extern "C" i32 MeshGen_NumVertsForQuads(i32 numQuads)
{
	return numQuads * MESH_GEN_VERTS_PER_QUAD;
}

extern "C" void MeshGen_ResetQuad(ZEQuad q, Vec2 size)
{
	Vec3 halfSize =
	{
		size.x * 0.5f,
		size.y * 0.5f
	};

	q.verts[0] = { -halfSize.x, -halfSize.y, 0 };
	q.verts[1] = { halfSize.x, -halfSize.y, 0 };
	q.verts[2] = { halfSize.x, halfSize.y, 0 };
	
	q.verts[3] = { -halfSize.x, -halfSize.y, 0 };
	q.verts[4] = { halfSize.x, halfSize.y, 0 };
	q.verts[5] = { -halfSize.x, halfSize.y, 0 };
	
	q.uvs[0] = { 0, 0 };
	q.uvs[1] = { 1, 0 };
	q.uvs[2] = { 1, 1 };
	
	q.uvs[3] = { 0, 0 };
	q.uvs[4] = { 1, 1 };
	q.uvs[5] = { 0, 1 };
	
	q.normals[0] = { 0, 0, -1 };
	q.normals[1] = { 0, 0, -1 };
	q.normals[2] = { 0, 0, -1 };
	
	q.normals[3] = { 0, 0, -1 };
	q.normals[4] = { 0, 0, -1 };
	q.normals[5] = { 0, 0, -1 };
}

/**
 * Assumes the quad is not distorted and perfectly rectangular!
 */
extern "C" void MeshGen_SetSquareUVs(ZEQuad q, i32 pixPerMetre, i32 texSize)
{
	f32 width = Vec3_Distance(q.verts[1], q.verts[0]);
	f32 height = Vec3_Distance(q.verts[2], q.verts[1]);
	printf("Quad w/h: %.3f, %.3f\n", width, height);
	/*
	if pix per metre == 32 and texture size = 128
	then 4 world units to cover the whole texture
	eg width 2, pixPerMetre 32, tex Size 128 ==
	pixCovered
	*/
	i32 pixStepX = (i32)((f32)pixPerMetre * width);
	i32 pixStepY = (i32)((f32)pixPerMetre * height);
	printf("PixStep %d, %d\n", pixStepX, pixStepY);
	f32 minX, maxX, minY, maxY;
	minX = 0;
	minY = 0;
	maxX = (f32)((f32)width / (f32)pixPerMetre) * (f32)pixStepX,
	maxY = (f32)((f32)height / (f32)pixPerMetre) * (f32)pixStepY;
	printf("Set Rect UVS : %.3f, %.3f - %3f, %.3f\n",
		minX, minY, maxX, maxY);
	
	q.uvs[0] = { minX, minY };
	q.uvs[1] = { maxX, minY };
	q.uvs[2] = { maxX, maxY };
	
	q.uvs[3] = { minX, minY };
	q.uvs[4] = { maxX, maxY };
	q.uvs[5] = { minX, maxY };
}

extern "C" ZEQuad MeshGen_SelectQuad(MeshData data, i32 quadIndex)
{
	ZEQuad q = {};
	// floats per quad
	i32 floatPerVert = MESH_GEN_VERTS_PER_QUAD * 3;
	i32 floatPerUV = MESH_GEN_VERTS_PER_QUAD * 2;
	q.verts = (Vec3*)(data.verts + (floatPerVert * quadIndex));
	q.uvs = (Vec2*)(data.uvs + (floatPerUV * quadIndex));
	q.normals = (Vec3*)(data.normals + (floatPerVert * quadIndex));
	return q;
}

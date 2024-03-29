#include "../ze_opengl_internal.h"

#define DEFAULT_LINE_THICKNESS 0.05f

/**
 * TODO - Move - If this function breaks the line down into mesh draw jobs...
 * why not do this in group_draw_items instead of the renderer and
 * spit out a big bunch of mesh jobs...?
 */
internal void ZRGL_DrawLineSegment(
	ZRDrawCmdMesh* meshCmd, Vec3 a, Vec3 b, M4x4* view, M4x4* projection)
{
	// Vec3 a = cmd->verts[0].pos;
	// Vec3 b = cmd->verts[1].pos;
	Vec3 halfSize = {(b.x - a.x) / 2.f, (b.y - a.y) / 2.f, (b.z - a.z) / 2.f};
	halfSize.x = ZAbsf(halfSize.x);
	halfSize.y = ZAbsf(halfSize.y);
	halfSize.z = ZAbsf(halfSize.z);
	f32 length = Vec3_Distance(a, b);
	Vec3 linePos;
	linePos.x = a.x + halfSize.x;
	linePos.y = a.y + halfSize.y;
	linePos.z = a.z + halfSize.z;
	Vec3 scale = { DEFAULT_LINE_THICKNESS, DEFAULT_LINE_THICKNESS, length };
	meshCmd->obj.t.scale = scale;
	Vec3 euler = Vec3_EulerAnglesBetween(a, b);
	euler.x = -euler.x;
	Transform_SetRotation(&meshCmd->obj.t, euler.x, euler.y, euler.z);
	// M3x3_Rot
	#if 0 // debugging because I suck.
	printf("Drawing line from %.3f, %.3f, %.3f to %.3f, %.3f, %.3f\n",
		a.x, a.y, a.z, b.x, b.y, b.z);
	printf("Drawing line centred at %.3f, %.3f, %.3f\n",
		linePos.x, linePos.y, linePos.z);
	printf("Scale %.3f, %.3f, %.3f\n", scale.x, scale.y, scale.z);
	printf("Line Euler: %.3f, %.3f, %.3f\n",
		euler.x * RAD2DEG, euler.y * RAD2DEG, euler.z * RAD2DEG);
	#endif
	// linePos
	meshCmd->obj.t.pos = linePos;
	ZRGL_DrawMesh(meshCmd, view, projection);
}

ze_external void ZRGL_DrawDebugLines(
	ZRDrawCmdDebugLines* cmd, M4x4* view, M4x4* projection)
{
	if (cmd == NULL) { return; }
	if (cmd->numVerts < 2) { return; }
	i32 matId = ZAssets_GetMaterialByName(FALLBACK_CHEQUER_MATERIAL)->header.id;
	ZRMeshAsset* mesh = ZAssets_GetMeshByName("cube");
	// printf("Draw lines\n");
	ZRDrawCmdMesh meshCmd = {};
	meshCmd.obj.data.SetAsMesh(mesh->header.id, matId);
	Transform_SetToIdentity(&meshCmd.obj.t);
	// meshCmd.obj.t.scale = { 0.1f, 0.1f, 10.f };

	// choose point iteration
	if (cmd->bChained == YES)
	{
		i32 numSegments = 0;
		if (numSegments % 2 == 0)
		{
			// eg 4 verts would be 3 segments
			numSegments = (cmd->numVerts / 2) + 1;
		}
		else
		{
			// eg 3 verts would be 2 segments
			numSegments = (cmd->numVerts + 1) / 2;
		}
		// printf("Draw chain with %d segments\n", numSegments);
		for (i32 i = 0; i < numSegments; ++i)
		{
			ZRLineVertex a = cmd->verts[i];
			ZRLineVertex b = cmd->verts[i + 1];
			// printf("Draw Chain segment %d -> %d from %.3f, %.3f, %.3f to %.3f, %.3f, %.3f\n",
			// 	   i, i + 1, a.pos.x, a.pos.y, a.pos.z, b.pos.x, b.pos.y, b.pos.z);
			ZRGL_DrawLineSegment(&meshCmd, a.pos, b.pos, view, projection);
		}
	}
	else
	{
		i32 stop = cmd->numVerts;
		if (stop % 2 != 0)
		{
			stop -= 1;
		}
		for (i32 i = 0; i < stop; i += 2)
		{
			ZRLineVertex a = cmd->verts[i];
			ZRLineVertex b = cmd->verts[i + 1];
			ZRGL_DrawLineSegment(&meshCmd, a.pos, b.pos, view, projection);
		}
	}
}

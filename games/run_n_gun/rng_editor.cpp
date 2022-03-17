#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_edScene;

ze_internal void BuildGrid()
{
	i32 gridLines = 24;
	i32 gridVertCount = gridLines * gridLines * 3;
	ZRMeshAsset* grid = g_engine.assets.AllocEmptyMesh("grid", gridVertCount);
	const f32 lineSize = 0.02f;

	// vertical
	Vec3 bl = { -lineSize, -8, 0 };
	Vec3 br = { lineSize, -8, 0 }; 
	Vec3 tl = { -lineSize, 8, 0 }; 
	Vec3 tr = { lineSize, 8, 0 };
	Vec3 n = { 0, 0, -1 };
	Vec3 origin = { -12.f, 0.f };
	for (i32 i = 0; i < gridLines; ++i)
	{
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(br, origin), Vec3_Add(tr, origin), {}, {}, {}, n, n, n);
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(tr, origin), Vec3_Add(tl, origin), {}, {}, {}, n, n, n);
		origin.x += 1.f;
	}
	// horizontal
	bl = { -12, -lineSize, 0 };
	br = { 12, -lineSize, 0 }; 
	tl = { -12, lineSize, 0 }; 
	tr = { 12, lineSize, 0 };
	origin = { 0.f, -12.f };
	for (i32 i = 0; i < gridLines; ++i)
	{
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(br, origin), Vec3_Add(tr, origin), {}, {}, {}, n, n, n);
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(tr, origin), Vec3_Add(tl, origin), {}, {}, {}, n, n, n);
		origin.y += 1.f;
	}
	
	ZRDrawObj* gridObj = g_engine.scenes.AddObject(g_edScene);
	Transform_SetToIdentity(&gridObj->t);
	i32 matId = g_engine.assets.GetMaterialByName(FALLBACK_MATERIAL_NAME)->header.id;
	gridObj->data.SetAsMesh(grid->header.id, matId);
	RNGPRINT("Grid draw obj Id %d, mat Id %d\n", gridObj->id, matId);
}

ze_external void Ed_Enable()
{
    u32 flags = g_engine.scenes.GetSceneFlags(g_edScene);
    flags &= ~ZSCENE_FLAG_NO_DRAW;
    g_engine.scenes.SetSceneFlags(g_edScene, flags);
}

ze_external void Ed_Disable()
{
    u32 flags = g_engine.scenes.GetSceneFlags(g_edScene);
    flags |= ZSCENE_FLAG_NO_DRAW;
    g_engine.scenes.SetSceneFlags(g_edScene, flags);
}

ze_external void Ed_Tick(ZEFrameTimeInfo timing)
{

}

ze_external void Ed_Init(ZEngine engine)
{
    g_engine = engine;
    g_edScene = g_engine.scenes.AddScene(0, 4096, 0);
    g_engine.scenes.ApplyDefaultOrthoProjection(g_edScene, 8, 16.f / 9.f);
    BuildGrid();
    Ed_Disable();
}

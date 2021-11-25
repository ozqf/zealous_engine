#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;
ze_internal i32 g_staticSceneIndex = -1;

ze_internal ZEBuffer g_frames;

internal ZEBlobStore g_entities;
internal i32 g_nextEntId = 1;

struct FrameHeader
{
	i32 sequence;
	i32 staticSceneIndex;
	i32 size;
};

ze_internal void AddStatic(Vec2 pos, Vec2 size)
{
	ZRDrawObj *platform = g_engine.scenes.AddFullTextureQuad(g_scene, TEX_PLATFORM, {size.x * 0.5f, size.y * 0.5f});
	CREATE_ENT_PTR(ent, platform)
	ent->type = ENT_TYPE_STATIC;
	ent->bodyId = ZP_AddStaticVolume(pos, size);
	platform->t.pos = Vec3_FromVec2(ZP_GetBodyPosition(ent->bodyId).pos, platform->t.pos.z);
	printf("Platform %d assigned body %d\n", platform->id, ent->bodyId);
}

ze_internal void AddDebris(Vec3 pos)
{
	ZRDrawObj *debris = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
	debris->t.pos = pos;
	CREATE_ENT_PTR(ent, debris)
	ent->type = ENT_TYPE_DEBRIS;
	ZPShapeDef def = {};
	def.pos = Vec2_FromVec3(pos);
	def.size = { 1, 1, };
	def.friction = 0;
	def.resitition = 0;
	ent->bodyId = ZP_AddDynamicVolume(def);
	printf("Added debris body %d at %.3f, %.3f\n", ent->bodyId, pos.x, pos.y);
}

ze_external void Sim_SyncDrawObjects()
{
	i32 numObjects = g_engine.scenes.GetObjectCount(g_scene);
	for (i32 i = 0; i < numObjects; ++i)
	{
		ZRDrawObj* obj = g_engine.scenes.GetObjectByIndex(g_scene, i);
		if (obj == NULL) { continue; }
		
		if (obj->data.type == ZR_DRAWOBJ_TYPE_NONE) { continue; }
		
		CREATE_ENT_PTR(ent, obj)
		if (ent->type == 0) { continue; }
		
		if (ent->bodyId == 0) { continue; }
		
		BodyState state = ZP_GetBodyState(ent->bodyId);
		obj->t.pos.x = state.t.pos.x;
		obj->t.pos.y = state.t.pos.y;
	}
}

ze_external void Sim_RestoreStaticScene(i32 index)
{
	if (index == g_staticSceneIndex)
	{
		return;
	}
	
	// cleanup scene here.
	
	g_staticSceneIndex = index;
	
	AddStatic({ 0, -1 }, { 8, 1 });
	
	AddStatic({ 0, -4 }, { 8, 1 });

	AddStatic({ 0, -8 }, { 24, 1 });
	AddStatic({ 0, 8 }, { 24, 1 });
	
	AddStatic({ -12, 0 }, { 1, 16 });
	AddStatic({ 12, 0 }, { 1, 16 });
}


ze_internal void Sim_RestoreEntity(EntStateHeader* header)
{
	switch (header->type)
	{
		case ENT_TYPE_DEBRIS:
		{
			DebrisEntState* debris = (DebrisEntState*)header;
			printf("Restore debris at %.3f, %.3f\n",
				debris->pos.x, debris->pos.y);
		} break;
		default:
			printf("Cannot restore unknown entity type %d\n", header->type);
			return;
	}
	
}

ze_external void Sim_RestoreFrame()
{
	
}

internal void Sim_WriteEntity(ZEBuffer* buf)
{
	
}

ze_external void Sim_WriteFrame()
{
	
}

ze_external void Sim_TickForward()
{
	Sim_SyncDrawObjects();
}

ze_external void Sim_TickBackward()
{
	Sim_SyncDrawObjects();
}

ze_external void Sim_Init(ZEngine engine, zeHandle sceneId)
{
	g_engine = engine;
	g_scene = sceneId;
	
	g_frames = Buf_FromMalloc(g_engine.system.Malloc, MegaBytes(1));
	
	Sim_RestoreStaticScene(0);
	
	DebrisEntState debris = {};
	debris.header.type = ENT_TYPE_DEBRIS;
	debris.header.numBytes = sizeof(DebrisEntState);
	
	for (i32 i = 0; i < 20; ++i)
	{
		// debris.pos.x = RANDF_RANGE(-10, 10);
		// debris.pos.y = RANDF_RANGE(1, 5);
		// Sim_RestoreEntity(&debris.header);
		f32 x = RANDF_RANGE(-10, 10);
		f32 y = RANDF_RANGE(1, 5);
		f32 depth = 0;
		AddDebris({x, y, depth});
	}
}

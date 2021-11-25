#include "rng_internal.h"

struct WorldVolume
{
	zeHandle bodyId;
	zeHandle drawObjId;
};

struct FrameHeader
{
	i32 sequence;
	i32 staticSceneIndex;
	i32 size;
};

#define WORLD_VOLUMES_MAX 1024

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

// static world
ze_internal i32 g_staticSceneIndex = -1;
ze_internal WorldVolume g_worldVolumes[WORLD_VOLUMES_MAX];
ze_internal i32 g_numWorldVolumes = 0;

ze_internal ZEBuffer g_frames;

internal ZEBlobStore g_entities;
internal i32 g_nextDynamicId = 1;
internal i32 g_nextStaticId = -1;

ze_internal i32 ReserveDynamicIds(i32 count)
{
	ZE_ASSERT(count > 0, "Reserve Id count must be > 0")
	i32 result = g_nextDynamicId;
	g_nextDynamicId += count;
	return result;
}

ze_internal i32 ReserveStaticIds(i32 count)
{
	ZE_ASSERT(count > 0, "Reserve Id count must be > 0")
	i32 result = g_nextStaticId;
	g_nextStaticId -= count;
	return result;
}

ze_internal Ent2d* GetFreeEntity(i32 id)
{
	Ent2d* ent = (Ent2d*)g_entities.GetFreeSlot(id);
	ZE_ASSERT(ent != NULL, "No free entities")
	return ent;
}

ze_internal WorldVolume* GetFreeWorldVolume()
{
	WorldVolume* vol = &g_worldVolumes[g_numWorldVolumes];
	g_numWorldVolumes += 1;
	return vol;
}

ze_internal void AddStatic(Vec2 pos, Vec2 size)
{
	WorldVolume* vol = GetFreeWorldVolume();
	ZRDrawObj *platform = g_engine.scenes.AddFullTextureQuad(g_scene, TEX_PLATFORM, {size.x * 0.5f, size.y * 0.5f});
	platform->t.pos = Vec3_FromVec2(pos, 0);
	vol->drawObjId = platform->id;
	vol->bodyId = ZP_AddStaticVolume(pos, size);
	printf("Platform %d assigned body %d\n", platform->id, vol->bodyId);
}

ze_external void Sim_RestoreStaticScene(i32 index)
{
	if (index == g_staticSceneIndex)
	{
		return;
	}
	
	// cleanup previous scene here!
	
	g_staticSceneIndex = index;
	
	AddStatic({ 0, -1 }, { 8, 1 });
	
	AddStatic({ 0, -4 }, { 8, 1 });

	AddStatic({ 0, -8 }, { 24, 1 });
	AddStatic({ 0, 8 }, { 24, 1 });
	
	AddStatic({ -12, 0 }, { 1, 16 });
	AddStatic({ 12, 0 }, { 1, 16 });
}

ze_internal void RestoreDebris(DebrisEntState* state)
{
	Ent2d* ent = (Ent2d*)g_entities.GetById(state->header.id);
	if (ent != NULL)
	{
		// restore state
	}
	else
	{
		// add entity
		ent = GetFreeEntity(state->header.id);
		ent->type = ENT_TYPE_DEBRIS;

		// add sprite
		ZRDrawObj *debris = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
		ent->drawId = debris->id;
		debris->t.pos = Vec3_FromVec2(state->pos, state->depth);

		// add body
		ZPShapeDef def = {};
		def.pos = state->pos;
		def.size = { 1, 1, };
		def.friction = 0;
		def.resitition = 0;
		ent->bodyId = ZP_AddDynamicVolume(def);
		printf("Added debris body %d at %.3f, %.3f\n",
			ent->bodyId, def.pos.x, def.pos.y);
	}
}

ze_external void Sim_SyncDrawObjects()
{
	i32 numEnts = g_entities.Count();
	for (i32 i = 0; i < numEnts; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		if (ent == NULL || ent->drawId == 0 || ent->bodyId == 0) { continue; }

		ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, ent->drawId);
		if (obj == NULL) { continue; }
		
		BodyState state = ZP_GetBodyState(ent->bodyId);
		obj->t.pos.x = state.t.pos.x;
		obj->t.pos.y = state.t.pos.y;

	}

	/*i32 numObjects = g_engine.scenes.GetObjectCount(g_scene);
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
	}*/
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
			RestoreDebris(debris);
			
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
	ZE_InitBlobStore(g_engine.system.Malloc, &g_entities, 1024, sizeof(Ent2d), 0);

	Sim_RestoreStaticScene(0);
	
	DebrisEntState debris = {};
	debris.header.type = ENT_TYPE_DEBRIS;
	debris.header.numBytes = sizeof(DebrisEntState);
	
	for (i32 i = 0; i < 20; ++i)
	{
		debris.header.id = ReserveDynamicIds(1);
		debris.pos.x = RANDF_RANGE(-10, 10);
		debris.pos.y = RANDF_RANGE(1, 5);
		debris.depth = 0;
		Sim_RestoreEntity(&debris.header);
		// f32 x = RANDF_RANGE(-10, 10);
		// f32 y = RANDF_RANGE(1, 5);
		// f32 depth = 0;
		// AddDebris({x, y, depth});
	}
}
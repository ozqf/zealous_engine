#include "rng_internal.h"

struct WorldVolume
{
	zeHandle bodyId;
	zeHandle drawObjId;
};

/*
FrameHeader
Ents
FrameFooter

*/
struct FrameHeader
{
	u32 sentinel;
	i32 sequence;
	// memory offset from start of this header struct
	zeSize offsetToEnts;
	// total bytes taken by entity chunk
	zeSize entBytes;
	i32 staticSceneIndex;
	// size is total bytes including head and footer structs
	zeSize size;
};

struct FrameFooter
{
	u32 sentinel;
	// size should match size in header
	zeSize size;
};

#define WORLD_VOLUMES_MAX 1024

ze_internal u32 g_restoreTick = 1;

ze_internal EntityType g_types[ENT_TYPE__LAST];

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

// static world
ze_internal i32 g_staticSceneIndex = -1;
ze_internal WorldVolume g_worldVolumes[WORLD_VOLUMES_MAX];
ze_internal i32 g_numWorldVolumes = 0;

ze_internal ZEBuffer g_frames;
// frame 0 is initial state. 1 is the first frame actually run
ze_internal u32 g_currentFrame = 0;
// last frame that has been stored in the frames buffer
ze_internal u32 g_lastFrame = 0;
ze_internal FrameHeader* g_currentHeader = NULL;

ze_internal ZEBlobStore g_entities;
ze_internal i32 g_nextDynamicId = 1;
ze_internal i32 g_nextStaticId = -1;

ze_internal ZEBuffer g_debugText;

ze_external ZEngine GetEngine()
{
	return g_engine;
}

ze_external zeHandle GetGameScene()
{
	return g_scene;
}

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

ze_external Ent2d* Sim_GetFreeEntity(i32 id)
{
	Ent2d* ent = (Ent2d*)g_entities.GetFreeSlot(id);
	ZE_ASSERT(ent != NULL, "No free entities")
	return ent;
}

ze_external Ent2d* Sim_GetEntById(i32 id)
{
	return (Ent2d*)g_entities.GetById(id);
}

/*
Function to cleanup basic entity components.
Should only be called from within an EntityType's remove function.
Remove function should clear the entity type to confirm it is deleted
*/
ze_external void Sim_RemoveEntityBase(Ent2d* ent)
{
	if (ent == NULL) { return; }
	if (ent->type != ENT_TYPE_NONE)
	{
		g_engine.system.Fatal("Attempting to remove entity with concrete type");
	}
	g_entities.MarkForRemoval(ent->id);
}

/*
Call this when wanting to remove an entity - calls type specific
remove function.
*/
ze_external void Sim_RemoveEntity(Ent2d* ent)
{
	if (ent == NULL) { return; }
	
	EntityType* type = Sim_GetEntityType(ent->type);
	ZE_ASSERT(type != NULL, "Null Entity type")
	ZE_ASSERT(type->Remove != NULL, "Entity Type has NULL Remove func")
	type->Remove(ent);
}

ze_external EntityType* Sim_GetEntityType(i32 typeId)
{
	ZE_ASSERT(typeId >= 0, "Type id < than zero")
	ZE_ASSERT(typeId < ENT_TYPE__LAST, "Type Id > max")
	return &g_types[typeId];
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

/////////////////////////////////////////////////
// read frame data
/////////////////////////////////////////////////

ze_internal void Sim_RestoreEntity(EntStateHeader* header, u32 restoreTick)
{
	EntityType* entType = &g_types[header->type];
	entType->Restore(header, restoreTick);
}

ze_external void Sim_RestoreFrame(FrameHeader* header)
{
	zeSize entityBytes = header->size - (sizeof(FrameHeader) + sizeof(FrameFooter));
	u8* read = (u8*)header + sizeof(FrameHeader);
	u8* end = read + entityBytes;
	
	if (g_staticSceneIndex != header->staticSceneIndex)
	{
		Sim_RestoreStaticScene(header->staticSceneIndex);
	}

	// firstly, iterate frame state data.
	// ents that are restored will be marked with the current restore tick.
	g_restoreTick += 1;
	while(read < end)
	{
		EntStateHeader* entHeader = (EntStateHeader*)read;
		read += entHeader->numBytes;
		Sim_RestoreEntity(entHeader, g_restoreTick);
	}

	// iterate ents list and remove any entities not marked
	// with the current restore tick.
	i32 numEnts = g_entities.Count();
	for (i32 i = 0; i < numEnts; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		if (ent == NULL) { continue; }

		if (ent->lastRestoreTick != g_restoreTick)
		{
			Sim_RemoveEntity(ent);
			// g_entities.MarkForRemoval(ent->id);
		}
	}
	g_entities.Truncate();
}

/////////////////////////////////////////////////
// write frame data
/////////////////////////////////////////////////

ze_external FrameHeader* Sim_WriteFrame(ZEBuffer* buf, i32 frameNumber)
{
	i8* start = buf->cursor;
	ZE_BUF_INIT_PTR_IN_PLACE(header, FrameHeader, buf)
	header->staticSceneIndex = g_staticSceneIndex;
	header->sequence = frameNumber;
	header->offsetToEnts = buf->cursor - start;
	
	i32 numEnts = g_entities.Count();
	i32 entsWritten = 0;
	i8* entsStart = buf->cursor;
	for (i32 i = 0; i < numEnts; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		if (ent == NULL) { continue; }
		
		EntityType* entType = &g_types[ent->type];
		entType->Write(ent, buf);
		entsWritten += 1;
	}
	header->entBytes = buf->cursor - entsStart;
	
	// footer
	ZE_BUF_INIT_PTR_IN_PLACE(footer, FrameFooter, buf)
	// -- no more added beyond this point --
	i8* end = buf->cursor;
	header->size = end - start;
	footer->size = header->size;
	// printf("Wrote %zd bytes to frame %d\n", header->size, header->sequence);
	return header;
	// printf("Wrote frame %d: %d ents, %.3fKB\n",
		// frameNumber, entsWritten, (f32)bytesWritten / 1024.f);
}

ze_internal FrameHeader* FindFrame(ZEBuffer* frames, i32 index)
{
	printf("Find frame %d...", index);
	i8* read = frames->start;
	i8* end = frames->cursor;
	while (read < end)
	{
		FrameHeader* header = (FrameHeader*)read;
		if (header->sequence == index)
		{
			printf(" at %lld\n", (u64)header);
			return header;
		}
		read += header->size;
	}
	printf(" failed!\n");
	return NULL;
}

ze_external void Sim_SpawnDebris(Vec2 pos)
{
	DebrisEntState debris = {};
	debris.header.type = ENT_TYPE_DEBRIS;
	debris.header.numBytes = sizeof(DebrisEntState);
	debris.header.id = ReserveDynamicIds(1);
	debris.pos = pos;
	debris.depth = 0;
	Sim_GetEntityType(ENT_TYPE_DEBRIS)->Restore(&debris.header, g_restoreTick);
}

ze_internal FrameHeader* WriteNewSession(ZEBuffer* frames)
{
	// --- clear frames array ---
	frames->Clear(NO);
	// --- setup sim state ---
	
	// static scene
	Sim_RestoreStaticScene(0);
	
	// add ents
	for (i32 i = 0; i < 0; ++i)
	{
		Vec2 pos = {};
		pos.x = RANDF_RANGE(-10, 10);
		pos.y = RANDF_RANGE(1, 5);
		Sim_SpawnDebris(pos);
	}
	
	FrameHeader* header = Sim_WriteFrame(frames, 0);
	return header;
}

ze_external void Sim_SyncDrawObjToPhysicsObj(zeHandle drawId, zeHandle bodyId)
{
	if (drawId == 0 || bodyId == 0) { return; }
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, drawId);
	BodyState state = ZP_GetBodyState(bodyId);
	obj->t.pos.x = state.t.pos.x;
	obj->t.pos.y = state.t.pos.y;
	Transform_SetRotation(&obj->t, 0, 0, state.t.radians);
}

ze_external void Sim_SyncDrawObjects()
{
	i32 numEnts = g_entities.Count();
	for (i32 i = 0; i < numEnts; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		if (ent == NULL) { continue; }

		EntityType* type = Sim_GetEntityType(ent->type);
		if (type != NULL && type->Sync != NULL)
		{
			type->Sync(ent);
		}
	}
}

ze_internal void UpdateDebugText()
{
	g_debugText.Clear(YES);
	
	i32 numChars = sprintf_s(
		(char*)g_debugText.cursor,
		g_debugText.Space(),
		
		"Frame %d. Last %d.\nFrame buffer capacity %zd of %zd (%.3f%%)",
		g_currentFrame,
		g_lastFrame,
		g_frames.Written(),
		g_frames.capacity,
		g_frames.PercentageUsed());
	
	g_debugText.cursor += numChars;
	// null terminate
	*g_debugText.cursor = '\0';
}

ze_external char* Sim_GetDebugText()
{
	return (char*)g_debugText.start;
}

ze_internal void TickEntities(f32 delta)
{
	i32 len = g_entities.Count();
	for (i32 i = 0; i < len; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		if (ent == NULL) { continue; }
		
		EntityType* type = &g_types[ent->type];
		if (type->Tick != NULL)
		{
			type->Tick(ent, delta);
		}
	}
}

ze_external void Sim_TickForward(f32 delta)
{
	g_currentFrame += 1;
	if (g_currentFrame > g_lastFrame)
	{
		// run a new frame
		TickEntities(delta);
		ZPhysicsTick(delta);
		Sim_WriteFrame(&g_frames, g_currentFrame);
		g_lastFrame += 1;
	}
	else
	{
		// restore frame
		FrameHeader* header = FindFrame(&g_frames, g_currentFrame);
		Sim_RestoreFrame(header);
	}
	Sim_SyncDrawObjects();
	UpdateDebugText();
}

ze_external void Sim_TickBackward(f32 delta)
{
	if (g_currentFrame == 0)
	{
		return;
	}
	g_currentFrame -= 1;
	FrameHeader* frame = FindFrame(&g_frames, g_currentFrame);
	Sim_RestoreFrame(frame);
	Sim_SyncDrawObjects();
	UpdateDebugText();
}

ze_internal void InitEntityTypes()
{
	EntityType* entType;
	
	entType	= &g_types[ENT_TYPE_DEBRIS];
	EntDebris_Register(entType);
	EntPlayer_Register(&g_types[ENT_TYPE_PLAYER]);
}

ze_external void Sim_DebugScanFrameData(i32 firstFrame, i32 maxFrames)
{
	const ZEBuffer* buf = &g_frames;
	if (firstFrame < 0)
	{
		firstFrame = g_lastFrame;
	}
	i32 count = 0;
	if (maxFrames == 0)
	{
		maxFrames = INT_MAX;
	}
	printf("--- Debug Scan Frames (%d max) ---\n", maxFrames);
	i8* read = buf->start;
	i8* end = buf->cursor;
	while (read < end)
	{
		if (count > maxFrames)
		{
			return;
		}
		count += 1;
		
		FrameHeader* fHead = (FrameHeader*)read;
		i8* entsRead = read + fHead->offsetToEnts;
		i8* entsEnd = entsRead + fHead->entBytes;
		read += fHead->size;
		
		// print header
		printf("Frame %d. %.3fKB total. %.3fKB of ents.\n",
			fHead->sequence, (f32)fHead->size / 1024.f, (f32)fHead->entBytes / 1024.f);
		
		// read entities within header.
		while (entsRead < entsEnd)
		{
			EntStateHeader* eHead = (EntStateHeader*)entsRead;
			entsRead += eHead->numBytes;
			printf("\tEnt %d. type %d. %dBytes\n",
				eHead->id, eHead->type, eHead->numBytes);
		}
	}
}

ze_external void Sim_Init(ZEngine engine, zeHandle sceneId)
{
	g_engine = engine;
	g_scene = sceneId;
	
	g_frames = Buf_FromMalloc(g_engine.system.Malloc, MegaBytes(1024));
	ZE_InitBlobStore(g_engine.system.Malloc, &g_entities, 1024, sizeof(Ent2d), 0);
	
	g_debugText = Buf_FromMalloc(g_engine.system.Malloc, MegaBytes(1));
	
	InitEntityTypes();

	FrameHeader* frame = WriteNewSession(&g_frames);
	Sim_DebugScanFrameData(0, 0);
	Sim_RestoreFrame(frame);
	
	g_currentFrame = 0;
	g_lastFrame = 0;
	
	UpdateDebugText();
}

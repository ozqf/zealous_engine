#include "rng_internal.h"
#include "../../plugins/ze_map2d.h"

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

ze_internal i32 g_bVerboseLoad = YES;

ze_internal EntityType g_types[ENT_TYPE__COUNT];

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;
ze_internal RNGTickInfo g_tickInfo;

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
// dynamic Ids increment, and an initial block of Ids is reserved
// for common stuff like the world or the player...
ze_internal i32 g_nextDynamicId = ENT_FIRST_DYNAMIC_ID;

// ...static Ids decrement. Static entities are created at
// world load time and NEVER move or change in ANY WAY!
ze_internal i32 g_nextStaticId = ENT_FIRST_STATIC_ID;

ze_internal ZEBuffer g_debugText;

ze_external FrameHeader* Sim_WriteFrame(ZEBuffer* buf, i32 frameNumber);
ze_external void Sim_RestoreFrame(FrameHeader* header);

ze_external ZEngine GetEngine()
{
	return g_engine;
}

ze_external zeHandle GetGameScene()
{
	return g_scene;
}

ze_external i32 Sim_GetRestoreTick()
{
	return g_restoreTick;
}

ze_external ZEBlobStore* Sim_GetEnts()
{
	return &g_entities;
}

ze_external void Sim_GetWorldVolumes(WorldVolume** vols, i32* count)
{
	*vols = g_worldVolumes;
	*count = g_numWorldVolumes;
}

ze_external Ent2d* Sim_FindPlayer()
{
	Ent2d* plyr = (Ent2d*)g_entities.GetById(ENT_RESERVED_ID_PLAYER);
	if (plyr == NULL) { return NULL; }
	ZE_ASSERT(plyr->type == ENT_TYPE_PLAYER, "Player entity is wrong type!")
	return plyr;
}

ze_external i32 Sim_GetPlayerStatus()
{
	Ent2d* plyr = Sim_FindPlayer();
	if (plyr == NULL)
	{
		return PLAYER_STATUS_NONE;
	}
	if (plyr->d.player.status < 0 || plyr->d.player.status > PLAYER_STATUS__LAST)
	{
		g_engine.system.Fatal("Invalid player status");
	}
	return plyr->d.player.status;
}

ze_external RNGTickInfo* Sim_GetTickInfo()
{
	return &g_tickInfo;
}

ze_external i32 Sim_ReserveDynamicIds(i32 count)
{
	ZE_ASSERT(count > 0, "Reserve Id count must be > 0")
	i32 result = g_nextDynamicId;
	g_nextDynamicId += count;
	return result;
}

ze_external i32 Sim_ReserveStaticIds(i32 count)
{
	ZE_ASSERT(count > 0, "Reserve Id count must be > 0")
	i32 result = g_nextStaticId;
	g_nextStaticId -= count;
	return result;
}

ze_external Ent2d* Sim_GetFreeEntity(i32 id, i32 type)
{
	Ent2d* ent;
	ent = Sim_GetEntById(id);
	ZE_ASSERT(ent == NULL, "Requested free ent already exists")
	ent = (Ent2d*)g_entities.GetFreeSlot(id);
	ZE_ASSERT(ent != NULL, "No free entities")
	ent->id = id;
	ent->type = type;
	ent->previousType = type;
	return ent;
}

ze_external Ent2d* Sim_GetEntById(i32 id)
{
	if (id == ENT_EMPTY_ID) { return NULL; }
	return (Ent2d*)g_entities.GetById(id);
}

ze_external i32 Sim_CheckLos(Vec2 a, Vec2 b)
{
	u16 mask = PHYSICS_LAYER_BIT_WORLD;
	const i32 maxResults = 16;
	i32 numResults = 0;
	ZPRaycastResult results[maxResults];
	numResults = ZP_Raycast(a, b, results, maxResults, mask);
	// RNGPRINT("Check LoS From %.3f, %.3f to %.3f, %.3f: %d\n",
	// 	a.x, a.y, b.x, b.y, numResults);
	return (numResults == 0);
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
	ZE_ASSERT(typeId < ENT_TYPE__COUNT, "Type Id > max")
	return &g_types[typeId];
}

///////////////////////////////////////////////////////
// static geometry creation
///////////////////////////////////////////////////////
ze_internal void ClearWorldVolumes()
{
	RNGPRINT("Clearing %d world volumes\n", g_numWorldVolumes);
	for (i32 i = 0; i < g_numWorldVolumes; ++i)
	{
		WorldVolume* vol = &g_worldVolumes[g_numWorldVolumes];
		ZP_RemoveBody(vol->bodyId);
		g_engine.scenes.RemoveObject(g_scene, vol->drawObjId);
	}
	g_numWorldVolumes = 0;
}

ze_internal WorldVolume* GetFreeWorldVolume()
{
	WorldVolume* vol = &g_worldVolumes[g_numWorldVolumes];
	g_numWorldVolumes += 1;
	return vol;
}

ze_internal void AddPlatform(Vec2 pos, f32 width)
{
	const f32 height = 0.2f;
	pos.y -= (height * 0.5f);

	WorldVolume* vol = GetFreeWorldVolume();
	vol->type = 1;
	ZRDrawObj *platform = g_engine.scenes.AddFullTextureQuad(
		g_scene,
		FALLBACK_TEXTURE_WHITE,
		{width * 0.5f, height * 0.5f},
		COLOUR_F32_LIGHT_GREY);
	platform->t.pos = Vec3_FromVec2(pos, -0.1f);
	vol->t = platform->t;
	vol->drawObjId = platform->id;
	u16 mask =
		PHYSICS_LAYER_BIT_PLATFORM |
		PHYSICS_LAYER_BIT_PLAYER |
		PHYSICS_LAYER_BIT_MOBS;
	vol->bodyId = ZP_AddStaticVolume(
		pos, { width, height }, PHYSICS_LAYER_BIT_PLATFORM, mask);
}

ze_internal void AddStatic(Vec2 pos, Vec2 size)
{
	if (g_bVerboseLoad)
	{
		RNGPRINT("Create world vol at %.3f, %.3f | %.3f by %.3f\n",
			pos.x, pos.y, size.x, size.y);
	}
	WorldVolume* vol = GetFreeWorldVolume();
	vol->type = 0;
	vol->t.pos = Vec3_FromVec2(pos, -0.1f);
	vol->t.scale = {size.x * 0.5f, size.y * 0.5f};
	ZRDrawObj *platform = g_engine.scenes.AddFullTextureQuad(
		g_scene,
		FALLBACK_TEXTURE_WHITE,
		{ vol->t.scale.x, vol->t.scale.y },
		COLOUR_F32_LIGHT_GREY);
	vol->drawObjId = platform->id;
	u16 mask = 
		PHYSICS_LAYER_BIT_WORLD |
		PHYSICS_LAYER_BIT_PLAYER |
		PHYSICS_LAYER_BIT_MOBS |
		PHYSICS_LAYER_BIT_DEBRIS;
	vol->bodyId = ZP_AddStaticVolume(
		pos, size, PHYSICS_LAYER_BIT_WORLD, mask);
	if (g_bVerboseLoad)
	{
		RNGPRINT("Platform %d assigned body %d\n", platform->id, vol->bodyId);
	}
}

ze_internal void AddStaticAABB(Point2 topLeftTile, Point2 size)
{
	if (size.x <= 0) { size.x = 1; }
	if (size.y <= 0) { size.y = 1; }
	Vec2 sizef = { f32(size.x), f32(size.y) };
	Vec2 pos;
	pos.x = f32(topLeftTile.x) + (sizef.x * 0.5f);
	pos.y = f32(topLeftTile.y) + (sizef.y * 0.5f);
	if (g_bVerboseLoad)
	{
		RNGPRINT("Create static AABB at %.3f, %.3f\n", pos.x, pos.y);
	}
	AddStatic(pos, sizef);
}

ze_external void Sim_RestoreStaticScene(i32 index)
{
	if (index == g_staticSceneIndex)
	{
		return;
	}
	
	// TODO: cleanup previous scene here!
	
	g_staticSceneIndex = index;
	RNGPRINT("Restore static scene %d\n", index);
	/*
	// AddStatic({ 0, -1 }, { 8, 1 });
	AddStaticAABB({ -4, -1 }, { 8, 1});
	// AddStatic({ 0, -4 }, { 8, 1 });
	AddStatic({ -9.5, -4 }, { 4, 1 });
	AddStatic({ 9.5, -4 }, { 4, 1 });

	AddPlatform({ 0, -4.5 }, 15);
	AddPlatform({ 0, -0.5 }, 15);

	// top and bottom border
	AddStatic({ 0, -8 }, { 24, 1 });
	AddStatic({ 0, 8 }, { 24, 1 });
	
	// left and right border
	AddStatic({ -12, 0 }, { 1, 16 });
	AddStatic({ 12, 0 }, { 1, 16 });
	*/
}

ze_internal void ReadMap2dEnt(Map2dReader* reader, Map2dEntity* mapEnt)
{
	char* typeStr = (char*)(reader->chars + mapEnt->typeStrOffset);
	//printf("Read typestr %s\n", typeStr);
	Vec2 pos = { mapEnt->pos.x, mapEnt->pos.y };
	if (ZStr_Equal(typeStr, "start"))
	{
		Sim_SpawnPlayer(pos);
	}
	else if (ZStr_Equal(typeStr, "spawner"))
	{
		Sim_SpawnSpawner(pos);
	}
	else if (ZStr_Equal(typeStr, "grunt"))
	{
		Sim_SpawnEnemyGrunt(pos, 0);
	}
}

///////////////////////////////////////////////////////
// start a new level
///////////////////////////////////////////////////////
ze_internal FrameHeader* WriteNewSession(ZEBuffer* frames, const char* mapName)
{
	// --- clear frames array ---
	frames->Clear(NO);
	// --- setup sim state ---
	i32 numEnts = g_entities.Count();
	for (i32 i = 0; i < numEnts; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		if (ent == NULL) { continue; }
		Sim_GetEntityType(ent->type)->Remove(ent);
	}
	g_entities.Truncate();
	
	// static scene
	// test read mapfile
	i32 mapIndex = atoi(mapName);
	RNGPRINT("Read map index %d\n", mapIndex);
	Map2d* mapData = Map2d_ReadEmbedded(mapIndex);
	if (g_bVerboseLoad)
	{
		Map2d_DebugDump(mapData);
	}
	Map2dReader reader = Map2d_CreateReader(mapData);

	ClearWorldVolumes();
	// volumes
	for (i32 i = 0; i < reader.numAABBs; ++i)
	{
		Map2dAABB* aabb = &reader.aabbs[i];
		Point2 min, max;
		min.x = (i32)aabb->min.x;
		min.y = (i32)aabb->min.y;
		max.x = (i32)aabb->max.x;
		max.y = (i32)aabb->max.y;
		Point2 size;
		size.x = ZAbsi(max.x - min.x) + 1;
		size.y = ZAbsi(max.y - min.y) + 1;
		if (aabb->type == 1)
		{
			// f32 halfWidth = (f32)size.x * 0.5f;
			// Vec2 pos = { (f32)min.x + halfWidth, (f32)max.y + 1.f };
			// AddPlatform(pos, (f32)size.x);
			continue;
		}
		else
		{
			AddStaticAABB(min, size);
		}
	}

	// lines
	for (i32 i = 0; i < reader.numLines; ++i)
	{
		Map2dLine* line = &reader.lines[i];
		f32 halfWidth = (line->b.x - line->a.x) / 2.f;
		Vec2 pos = line->a;
		pos.x += halfWidth;
		if (g_bVerboseLoad)
		{
			RNGPRINT("Add platform at %.3f, %3f, width %.3f\n",
				pos.x, pos.y, (halfWidth * 2.f));
		}
		AddPlatform(pos, halfWidth * 2.f);

	}

	// ents
	for (i32 i = 0; i < reader.numEnts; ++i)
	{
		Map2dEntity* ent = &reader.ents[i];
		if (g_bVerboseLoad)
		{
			RNGPRINT("Read ent type %s pos %.3f, %.3f\n",
				(reader.chars + ent->typeStrOffset), ent->pos.x, ent->pos.y);
		}
		ReadMap2dEnt(&reader, ent);
	}

	// free map
	reader = {};
	Map2d_Free(mapData);

	// Sim_RestoreStaticScene(0);

	
	const i32 numDebris = 0;
	// add ents
	for (i32 i = 0; i < numDebris; ++i)
	{
		Vec2 pos = {};
		pos.x = RANDF_RANGE(-10, 10);
		pos.y = RANDF_RANGE(1, 5);
		Sim_SpawnDebris(pos, {}, 0.f);
	}

	// spawn a player
	#if 0
	Sim_SpawnPlayer({0, -2});
	Sim_SpawnSpawner({-10, 4});
	Sim_SpawnSpawner({10, 4});
	#endif
	
	FrameHeader* header = Sim_WriteFrame(frames, 0);
	return header;
}

ze_external void Sim_StartNewGame(char* mapName)
{
	FrameHeader* frame = WriteNewSession(&g_frames, mapName);
	if (g_bVerboseLoad)
	{
		Sim_DebugScanFrameData(0, 0);
 	}
	Sim_RestoreFrame(frame);
	
	g_currentFrame = 0;
	g_lastFrame = 0;
}

/////////////////////////////////////////////////
// read frame data
/////////////////////////////////////////////////

ze_external void Sim_RestoreEntity(EntStateHeader* header)
{
	EntityType* entType = &g_types[header->type];
	entType->Restore(header, g_restoreTick);
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
		Sim_RestoreEntity(entHeader);
	}

	// iterate ents list and remove any entities not marked
	// with the current restore tick.
	i32 numEnts = g_entities.Count();
	for (i32 i = 0; i < numEnts; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		if (ent == NULL) { continue; }

		if (ent->lastRestoreFrame != g_restoreTick)
		{
			Sim_RemoveEntity(ent);
			// g_entities.MarkForRemoval(ent->id);
		}
	}
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
	// RNGPRINT("Wrote %zd bytes to frame %d\n", header->size, header->sequence);
	return header;
	// RNGPRINT("Wrote frame %d: %d ents, %.3fKB\n",
		// frameNumber, entsWritten, (f32)bytesWritten / 1024.f);
}

ze_internal FrameHeader* FindFrame(ZEBuffer* frames, i32 index)
{
	// Note: Linear search
	// RNGPRINT("Find frame %d...", index);
	i8* read = frames->start;
	i8* end = frames->cursor;
	while (read < end)
	{
		FrameHeader* header = (FrameHeader*)read;
		if (header->sequence == index)
		{
			// RNGPRINT(" at %lld\n", (u64)header);
			return header;
		}
		read += header->size;
	}
	RNGPRINT("Find frame failed!\n");
	return NULL;
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
	const char* controlsTxt = "WASD move. mouse aim/shoot. Shift stop/start.\nQ/E rewind/fastforward. Esc quit.";
	Vec2 playerPos = {};
	Ent2d* playerEnt = Sim_FindPlayer();
	if (playerEnt != NULL)
	{
		BodyState body = ZP_GetBodyState(playerEnt->d.player.physicsBodyId);
		playerPos = body.t.pos;
	}
	Vec2 aimPos = Sim_GetTickInfo()->cursorWorldPos;
	Point2 cursorTile = {};
	// cursorTile.x = int(aimPos.x);
	// cursorTile.y = int(aimPos.y);
	cursorTile.x = int(ZFLOORF(aimPos.x));
	cursorTile.y = int(ZFLOORF(aimPos.y));

	i32 numEnts = g_entities.Count();
	i32 maxEnts = g_entities.Capacity();
	f32 secondsRecorded = g_lastFrame / 60.f;
	f32 secondsToNow = g_currentFrame / 60.f;
	i32 playerStatus = Sim_GetPlayerStatus();
	ZPStats physics = ZPhysicsStats();

	g_debugText.Clear(YES);
	char* write = (char*)g_debugText.cursor;
	char* end = write + g_debugText.Space();
	write += sprintf_s(
		write,
		end - write,
		"Cursor tile %d,%d. Player pos %.3f, %.3f\n",
		cursorTile.x, cursorTile.y, playerPos.x, playerPos.y);
	
	write += sprintf_s(
		write,
		end - write,
		"Physics: Bodies %d, registered, dynamic: %d, static %d\n",
		physics.numBodies, physics.numRegisteredDynamic, physics.numRegisteredStatic);
	
	write += sprintf_s(
		write,
		end - write,
		"%s\nPlayer status %d. Ents %d of %d\nFrame %d. Last %d (%.3f of %.3f seconds).\nFrame buffer capacity %zdKB of %zdKB (%.3f%%)",
		controlsTxt,
		playerStatus,
		numEnts,
		maxEnts,
		g_currentFrame,
		g_lastFrame,
		secondsToNow,
		secondsRecorded,
		g_frames.Written() / 1024,
		g_frames.capacity / 1024,
		g_frames.PercentageUsed());
	
	g_debugText.cursor = (i8*)write;
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

ze_external void Sim_ClearFutureFrames()
{
	if (g_lastFrame > g_currentFrame)
	{
		g_lastFrame = g_currentFrame;
	}
	FrameHeader* frame = FindFrame(&g_frames, g_currentFrame);
	g_frames.cursor = (i8*)frame + frame->size;
}

ze_internal void PostTick()
{
	Sim_SyncDrawObjects();
	g_entities.Truncate();
	UpdateDebugText();
}

ze_external void Sim_TickForward(RNGTickInfo info, i32 bInteractive)
{
	// No ticking if player is dead!
	if (Sim_GetPlayerStatus() == PLAYER_STATUS_DEAD)
	{
		// g_entities.Truncate();
		PostTick();
		return;
	}
	g_tickInfo = info;
	g_currentFrame += 1;
	
	if (g_currentFrame > g_lastFrame)
	{
		// run a new frame?
		// if not interactive (ie, playing), skip this part, we are
		// at the end of the frames stored.
		// undo the frame progression and quit out.
		if (!bInteractive)
		{
			g_currentFrame--;
			return;
		}
		
		// read controls and feed to player
		EntPlayer_SetInput(info);
		
		// tick logic and physics
		TickEntities(info.delta);
		ZPhysicsTick(info.delta);
		// write output
		Sim_WriteFrame(&g_frames, g_currentFrame);
		// advance the count of the last frame stored
		g_lastFrame += 1;
	}
	else
	{
		// restore frame
		FrameHeader* header = FindFrame(&g_frames, g_currentFrame);
		Sim_RestoreFrame(header);
	}
	PostTick();
}

ze_external void Sim_TickBackward(RNGTickInfo info)
{
	g_tickInfo = info;
	// can't go further back if we're at the start...
	if (g_currentFrame == 0)
	{
		return;
	}
	g_currentFrame -= 1;
	FrameHeader* frame = FindFrame(&g_frames, g_currentFrame);
	Sim_RestoreFrame(frame);
	PostTick();
}

ze_internal void InitEntityTypes()
{
	RNGPRINT("Init entity types\n");
	for (i32 i = 0; i < ENT_TYPE__COUNT; ++i)
	{
		g_types[i] = {};
	}
	EntNull_Register(&g_types[ENT_TYPE_NONE]);
	EntDebris_Register(&g_types[ENT_TYPE_DEBRIS]);
	EntPlayer_Register(&g_types[ENT_TYPE_PLAYER]);
	EntGrunt_Register(&g_types[ENT_TYPE_ENEMY_GRUNT]);
	EntPointProjectile_Register(&g_types[ENT_TYPE_POINT_PRJ]);
	EntGfxSprite_Register(&g_types[ENT_TYPE_GFX_SPRITE]);
	EntSpawner_Register(&g_types[ENT_TYPE_SPAWNER]);
	RNGPRINT("Entity types initialised\n");
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
	RNGPRINT("--- Debug Scan Frames (%d max) ---\n", maxFrames);
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
		RNGPRINT("Frame %d. %.3fKB total. %.3fKB of ents.\n",
			fHead->sequence, (f32)fHead->size / 1024.f, (f32)fHead->entBytes / 1024.f);
		
		// read entities within header.
		while (entsRead < entsEnd)
		{
			EntStateHeader* eHead = (EntStateHeader*)entsRead;
			entsRead += eHead->numBytes;
			RNGPRINT("\tEnt %d. type %d. %dBytes\n",
				eHead->id, eHead->type, eHead->numBytes);
		}
	}
}

ze_internal void Sim_CrashDump()
{
	RNGPRINT("--- RNG Sim crash dump ---\n");
	i32 numEnts = g_entities.Count();
	for (i32 i = 0; i < numEnts; ++i)
	{
		Ent2d* ent = (Ent2d*)g_entities.GetByIndex(i);
		RNGPRINT("%d: id %d, type %d ",
			i, ent->id, ent->type);
		EntityType* type = Sim_GetEntityType(ent->type);
		if (type == NULL)
		{
			RNGPRINT("No type data!\n");
			continue;
		}
		RNGPRINT("(%s) ", type->label);
		if (type->Print != NULL)
		{
			type->Print(ent);
		}
		RNGPRINT("\n");
	}
}

ze_external void Sim_Init(ZEngine engine, zeHandle sceneId)
{
	g_engine = engine;
	g_scene = sceneId;
	ZE_SetFatalError(g_engine.system.Fatal);
	g_engine.system.RegisterCrashDumpFunction(Sim_CrashDump);
	
	g_frames = Buf_FromMalloc(g_engine.system.Malloc, MegaBytes(1024));
	ZE_InitBlobStore(g_engine.system.Malloc, &g_entities, ENTITY_COUNT, sizeof(Ent2d), 0);
	
	g_debugText = Buf_FromMalloc(g_engine.system.Malloc, MegaBytes(1));
	
	InitEntityTypes();
	Map2d_Init(engine);

	Sim_StartNewGame("3");

	UpdateDebugText();
}

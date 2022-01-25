#ifndef RNG_INTERNAL_H
#define RNG_INTERNAL_H

#include "../../headers/zengine.h"
#include "../../plugins/ze_physics2d.h"

#include "tile_map.h"

// for RAND
#include <stdlib.h>

#define TEX_PLATFORM "platform_texture"
#define TEX_CURSOR "cursor_texture"

#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"

#define ACCLE_FORCE 100
#define MOVE_SPEED 8

#define GAME_STATE_PLAYING 0
#define GAME_STATE_PAUSED 1

#define ENT_TYPE_NONE 0
#define ENT_TYPE_STATIC 1
#define ENT_TYPE_PLAYER 2
#define ENT_TYPE_DEBRIS 3
#define ENT_TYPE__LAST 4

#define ENTITY_COUNT 4096

#define RANDF ((f32)rand() / RAND_MAX)
#define RANDF_RANGE(minValueF, maxValueF) (RANDF * (maxValueF - minValueF) + minValueF)

// #define CREATE_ENT_PTR(entPtrName, drawObjPtr) \
// Ent2d* entPtrName = NULL; \
// if (drawObjPtr != NULL) { entPtrName = (Ent2d*)drawObjPtr->userData; }

struct Ent2d
{
	i32 id;
	i32 type;
	u32 lastRestoreTick;
    Vec2 velocity;
	f32 tick;
    // f32 degrees;
    // f32 rotDegreesPerSecond;
	zeHandle drawId = 0;
	zeHandle bodyId = 0;
};

struct EntStateHeader
{
	i32 id;
	i32 type;
	i32 numBytes;
};

struct DebrisEntState
{
	EntStateHeader header;
	Vec2 pos;
	f32 depth;
	f32 degrees;
	Vec2 velocity;
	f32 angularVelocity;
	f32 tick;
};

struct PlayerEntState
{
	EntStateHeader header;
	Vec2 pos;
	f32 depth;
	f32 degrees;
	Vec2 velocity;
	f32 tick;
};

struct EntityType
{
	i32 type;
	char* label;
	void (*Restore)(EntStateHeader* ptr, u32 restoreTick);
	void (*Write)(Ent2d* ent, ZEBuffer* buf);
	void (*Remove)(Ent2d* ent);
	void (*Tick)(Ent2d* ent, f32 delta);
};

struct RNGShared
{
	ZEngine engine;
	zeHandle scene;
	
};	

ze_external void Sim_Init(ZEngine engine, zeHandle sceneId);
ze_external char* Sim_GetDebugText();
ze_external void Sim_SyncDrawObjects();
ze_external void Sim_TickForward(f32 delta);
ze_external void Sim_TickBackward(f32 delta);
ze_external void Sim_SpawnDebris(Vec2 pos);

ze_external ZEngine GetEngine();
ze_external zeHandle GetGameScene();

// general entities
ze_external Ent2d* Sim_GetFreeEntity(i32 id);
ze_external Ent2d* Sim_GetEntById(i32 id);
ze_external EntityType* Sim_GetEntityType(i32 typeId);
ze_external void Sim_RemoveEntityBase(Ent2d* ent);
ze_external void Sim_RemoveEntity(Ent2d* ent);

// specific entities
ze_external void EntDebris_Register(EntityType* type);
ze_external void EntPlayer_Register(EntityType* type);

ze_external void Sim_DebugScanFrameData(i32 firstFrame, i32 maxFrames);

#endif // RNG_INTERNAL_H
 
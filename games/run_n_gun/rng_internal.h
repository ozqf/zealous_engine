#ifndef RNG_INTERNAL_H
#define RNG_INTERNAL_H

#include "../../headers/zengine.h"
#include "../../plugins/ze_physics2d.h"

#include "tile_map.h"

// for RAND
#include <stdlib.h>

#define RANDF ((f32)rand() / RAND_MAX)
#define RANDF_RANGE(minValueF, maxValueF) (RANDF * (maxValueF - minValueF) + minValueF)

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
#define ENT_TYPE_POINT_PRJ 4
#define ENT_TYPE__COUNT 5

#define ENTITY_COUNT 4096

// #define CREATE_ENT_PTR(entPtrName, drawObjPtr) \
// Ent2d* entPtrName = NULL; \
// if (drawObjPtr != NULL) { entPtrName = (Ent2d*)drawObjPtr->userData; }

struct EntStateHeader
{
	i32 id;
	i32 type;
	i32 numBytes;
};

struct EntDebris
{
	Vec2 velocity;
	f32 tick;
	zeHandle drawId = 0;
	zeHandle physicsBodyId = 0;
};

struct DebrisEntState
{
	EntStateHeader header;
	// Ent
	f32 tick;

	// Display
	f32 depth;

	// body
	Vec2 pos;
	f32 degrees;
	Vec2 velocity;
	f32 angularVelocity;
};

struct EntPlayer
{
	Vec2 velocity;
	f32 tick;
	f32 aimDegrees;
	u32 buttons;
	zeHandle bodyDrawId = 0;
	zeHandle gunDrawId = 0;
	zeHandle physicsBodyId = 0;
};

struct PlayerEntState
{
	EntStateHeader header;
	// ent
	f32 aimDegrees;
	u32 buttons;
	f32 tick;

	// Display
	f32 depth;

	// body
	Vec2 pos;
	Vec2 velocity;
};

struct EntPointProjectile
{
	Vec2 pos;
	f32 depth;
	f32 radians;
};

struct EntPointProjectileState
{
	EntStateHeader header;
	EntPointProjectile data;
};

union EntData
{
	EntPlayer player;
	EntDebris debris;
	EntPointProjectile pointPrj;
};

struct Ent2d
{
	i32 id;
	i32 type;
	u32 lastRestoreTick;
	f32 tick;
	EntData d;
};

struct EntityType
{
	i32 type;
	char* label;
	void (*Restore)(EntStateHeader* ptr, u32 restoreTick);
	void (*Write)(Ent2d* ent, ZEBuffer* buf);
	void (*Remove)(Ent2d* ent);
	void (*Tick)(Ent2d* ent, f32 delta);
	void (*Sync)(Ent2d* ent);
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
ze_external void Sim_SyncDrawObjToPhysicsObj(zeHandle drawId, zeHandle bodyId);

// specific entities
ze_external void EntNull_Register(EntityType* type);
ze_external void EntDebris_Register(EntityType* type);
ze_external void EntPlayer_Register(EntityType* type);
ze_external void EntPointProjectile_Register(EntityType* type);

ze_external void Sim_SpawnPointProjectile(Vec2 pos, f32 radians);

ze_external void Sim_DebugScanFrameData(i32 firstFrame, i32 maxFrames);

#endif // RNG_INTERNAL_H
 
#ifndef RNG_INTERNAL_H
#define RNG_INTERNAL_H

#include "../../headers/zengine.h"
#include "../../plugins/ze_physics2d.h"

#include "tile_map.h"

#define RNGPRINT(fmt, ...) \
printf(fmt, __VA_ARGS__)

// for RAND
#include <stdlib.h>

#define RANDF ((f32)rand() / RAND_MAX)
#define RANDF_RANGE(minValueF, maxValueF) (RANDF * (maxValueF - minValueF) + minValueF)

#define TEX_PLATFORM "platform_texture"
#define TEX_CURSOR "cursor_texture"

// actions
#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"

#define ACTION_ATTACK_1 "attack_1"
#define ACTION_ATTACK_2 "attack_2"
#define ACTION_USE "use"
#define ACTION_SPECIAL "special"
#define ACTION_TIME_FORWARD "forward"
#define ACTION_TIME_BACKWARD "backward"
#define ACTION_TIME_STOP "stop"
#define ACTION_TIME_PLAY "play"

// button bits
#define INPUT_BIT_LEFT (1 << 0)
#define INPUT_BIT_RIGHT (1 << 1)
#define INPUT_BIT_UP (1 << 2)
#define INPUT_BIT_DOWN (1 << 3)
#define INPUT_BIT_ATK_1 (1 << 4)
#define INPUT_BIT_ATK_2 (1 << 5)
#define INPUT_BIT_USE (1 << 6)

#define ACCLE_FORCE 100
#define MOVE_SPEED 8

#define GAME_STATE_PLAYING 0
#define GAME_STATE_PAUSED 1

#define TEAM_ID_NONE 0
#define TEAM_ID_PLAYER 1
#define TEAM_ID_ENEMY 2
#define TEAM_ID_NONCOMBATANT 3

#define ENT_TYPE_NONE 0
#define ENT_TYPE_STATIC 1
#define ENT_TYPE_PLAYER 2
#define ENT_TYPE_DEBRIS 3
#define ENT_TYPE_POINT_PRJ 4
#define ENT_TYPE__COUNT 5

// dynamic Ids increment, and an initial block of Ids is reserved
// for common stuff like the world or the player.
#define ENT_FIRST_DYNAMIC_ID 3
#define ENT_FIRST_STATIC_ID -1

// reserved Ids for specific entities.
#define ENT_RESERVED_ID_WORLD 1
#define ENT_RESERVED_ID_PLAYER 2

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

////////////////////////////////////////////////
// debris
////////////////////////////////////////////////
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

////////////////////////////////////////////////
// player
////////////////////////////////////////////////
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
	f32 tick;
	f32 aimDegrees;
	u32 buttons;

	// Display
	f32 depth;

	// body
	Vec2 pos;
	Vec2 velocity;
};

////////////////////////////////////////////////
// point projectile
////////////////////////////////////////////////
struct PointProjectileData
{
	Vec2 pos;
	f32 depth;
	f32 radians;
	i32 teamId;
	f32 tick;
};

struct PointProjectileComponents
{
	zeHandle drawId;
};

struct EntPointProjectile
{
	PointProjectileData data;
	PointProjectileComponents comp;
};

struct EntPointProjectileState
{
	EntStateHeader header;
	PointProjectileData data;
};

////////////////////////////////////////////////
// entity base
////////////////////////////////////////////////
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

////////////////////////////////////////////////
// misc
////////////////////////////////////////////////
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

struct RNGTickInfo
{
	float delta;
	u32 buttons;
	Vec2 cursorWorldPos;
	Vec2 cursorScreenPos;
};

ze_external void Sim_Init(ZEngine engine, zeHandle sceneId);
ze_external char* Sim_GetDebugText();
ze_external void Sim_SyncDrawObjects();
ze_external void Sim_TickForward(RNGTickInfo info);
ze_external void Sim_TickBackward(RNGTickInfo info);
ze_external RNGTickInfo* Sim_GetTickInfo();
ze_external void Sim_SpawnDebris(Vec2 pos);
ze_external void Sim_SpawnPlayer(Vec2 pos);
ze_external void Sim_SpawnProjectile(Vec2 pos, f32 degrees, i32 teamId);

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
ze_external void EntPlayer_SetInput(RNGTickInfo info);
ze_external void EntPointProjectile_Register(EntityType* type);

ze_external void Sim_SpawnPointProjectile(Vec2 pos, f32 radians);

ze_external void Sim_DebugScanFrameData(i32 firstFrame, i32 maxFrames);

#endif // RNG_INTERNAL_H
 
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

#define GAME_STATE_PLAYING 0
#define GAME_STATE_PAUSED 1

#define APP_STATE_GAME 0
#define APP_STATE_EDITOR 1

#define RNG_CMD_MENU_ON "rng main-menu-on"
#define RNG_CMD_MENU_OFF "rng main-menu-off"
#define RNG_CMD_GAME "rng game"
#define RNG_CMD_EDITOR "rng editor"
#define RNG_CMD_APP_TOGGLE "rng app-toggle"

#define SCENE_ORDER_GAME 0
#define SCENE_ORDER_UI 1
#define SCENE_ORDER_MENU 2

#define TEX_PLATFORM "platform_texture"
#define TEX_CURSOR "cursor_texture"

#define PHYSICS_LAYER_BIT_WORLD (1 << 0)
#define PHYSICS_LAYER_BIT_PLATFORM (1 << 1)
#define PHYSICS_LAYER_BIT_MOBS (1 << 2)
#define PHYSICS_LAYER_BIT_PLAYER (1 << 3)
#define PHYSICS_LAYER_BIT_DEBRIS (1 << 4)

#define PRJ_TEMPLATE_ENEMY_DEFAULT 0
#define PRJ_TEMPLATE_PLAYER_DEFAULT 1

#define SERIALISED

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
#define ACTION_TIME_FAST_FORWARD "fast_forward"
#define ACTION_TIME_FAST_REWIND "fast_rewind"

#define ACTION_MENU "menu"

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

#define TEAM_ID_NONE 0
#define TEAM_ID_PLAYER 1
#define TEAM_ID_ENEMY 2
#define TEAM_ID_NONCOMBATANT 3

#define ENT_EMPTY_TAG 0

#define ENT_TYPE_NONE 0
#define ENT_TYPE_STATIC 1
#define ENT_TYPE_PLAYER 2
#define ENT_TYPE_DEBRIS 3
#define ENT_TYPE_POINT_PRJ 4
#define ENT_TYPE_ENEMY_GRUNT 5
#define ENT_TYPE_GFX_SPRITE 6
#define ENT_TYPE_SPAWNER 7
#define ENT_TYPE__COUNT 8

#define ENT_HIT_RESPONSE_SOLID 0
#define ENT_HIT_RESPONSE_DAMAGED 1
#define ENT_HIT_RESPONSE_KILLED 2
#define ENT_HIT_RESPONSE_NONE 3

#define ENT_EMPTY_ID 0
// dynamic Ids increment, and an initial block of Ids is reserved
// for common stuff like the world or the player.
#define ENT_FIRST_DYNAMIC_ID 3
#define ENT_FIRST_STATIC_ID -1

// reserved Ids for specific entities.
#define ENT_RESERVED_ID_WORLD 1
#define ENT_RESERVED_ID_PLAYER 2

#define ENTITY_COUNT 4096

#define PLAYER_STATUS_NONE 0
#define PLAYER_STATUS_PLAYING 1
#define PLAYER_STATUS_DEAD 2
#define PLAYER_STATUS__LAST 2

#define GROUND_CHECK_MASK (PHYSICS_LAYER_BIT_WORLD | PHYSICS_LAYER_BIT_PLATFORM)

// #define CREATE_ENT_PTR(entPtrName, drawObjPtr) \
// Ent2d* entPtrName = NULL; \
// if (drawObjPtr != NULL) { entPtrName = (Ent2d*)drawObjPtr->userData; }

struct EntStateHeader
{
	i32 id;
	i32 tag;
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

SERIALISED struct DebrisEntSave
{
	EntStateHeader header;
	// Ent
	f32 tick;

	// Display - read from draw object z position
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
	// state
	Vec2 velocity;
	f32 tick;
	f32 aimDegrees;
	u32 buttons;
	i32 status;
	i32 touchFlags;

	// components
	zeHandle bodyDrawId = 0;
	zeHandle gunDrawId = 0;
	zeHandle physicsBodyId = 0;
};

struct PlayerEntSave
{
	EntStateHeader header;
	// ent
	f32 tick;
	f32 aimDegrees;
	u32 buttons;
	i32 status;
	i32 touchFlags;

	// Display
	f32 depth;

	// body
	Vec2 pos;
	Vec2 velocity;
};

////////////////////////////////////////////////
// enemy grunt
////////////////////////////////////////////////

struct EntGrunt
{
	// state
	i32 state;
	f32 tick;
	f32 aimDegrees;
	i32 targetId;
	i32 sourceId;
	i32 health;

	// components
	zeHandle physicsBodyId;
	zeHandle bodyDrawId;
	zeHandle gunDrawId;
};

struct EntGruntSave
{
	EntStateHeader header;
	// state
	i32 state;
	f32 tick;
	f32 aimDegrees;
	i32 targetId;
	i32 sourceId;
	i32 health;

	// component data
	Vec2 pos;
	f32 depth;
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
	i32 templateId;
	i32 sourceId;
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

struct EntPointProjectileSave
{
	EntStateHeader header;
	PointProjectileData data;
};

////////////////////////////////////////////////
// GFX Sprite
////////////////////////////////////////////////
struct GfxSpriteData
{
	Vec2 pos;
	f32 depth;
	f32 radians;
	f32 tick;
};

struct GfxSpriteSave
{
	EntStateHeader header;
	GfxSpriteData data;
};

struct EntGfxSprite
{
	GfxSpriteData data;
	zeHandle drawId;
};


////////////////////////////////////////////////
// Spawner
////////////////////////////////////////////////
struct EntSpawner
{
	Vec2 pos;
	i32 spawnType;
	f32 tick;
	i32 totalSpawns;
	i32 maxAlive;
	i32 alive;
	f32 delay;
};

struct SpawnerSave
{
	EntStateHeader header;
	EntSpawner data;
};

////////////////////////////////////////////////
// entity base
////////////////////////////////////////////////

// Concrete entity types
union EntData
{
	EntPlayer player;
	EntDebris debris;
	EntPointProjectile pointPrj;
	EntGrunt grunt;
	EntGfxSprite gfxSprite;
	EntSpawner spawner;
};

// base entity type
struct Ent2d
{
	i32 id;
	i32 type;
	i32 tag;
	// used to detect if an object was not included in
	// some restored frame data, and thus if it should be removed.
	u32 lastRestoreFrame;
	// for debugging cleanup
	i32 previousType;
	EntData d;
};

ze_internal EntStateHeader Ent_SaveHeaderFromEnt(
	Ent2d* ent,
	i32 numBytes)
{
	EntStateHeader h;
	h.id = ent->id;
	h.tag = ent->tag;
	h.type = ent->type;
	h.numBytes = numBytes;
	return h;
}

ze_internal EntStateHeader Ent_SaveHeaderFromRaw(
	i32 id,
	i32 tag,
	i32 type,
	i32 numBytes)
{
	EntStateHeader h;
	h.id = id;
	h.tag = tag;
	h.type = type;
	h.numBytes = numBytes;
	return h;
}

////////////////////////////////////////////////
// misc
////////////////////////////////////////////////

struct DamageHit
{
	i32 damage;
	i32 teamId;
	Vec2 pos;
	Vec2 normal;
	Vec2 dir;
};

struct EntHitResponse
{
	i32 responseType;
	i32 damageDone;
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
	EntHitResponse (*Hit)(Ent2d* victim, DamageHit* hit);
	void (*Print)(Ent2d* ent);
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
ze_external void Sim_TickForward(RNGTickInfo info, i32 bInteractive);
ze_external void Sim_TickBackward(RNGTickInfo info);
ze_external void Sim_ClearFutureFrames();
ze_external RNGTickInfo* Sim_GetTickInfo();
ze_external i32 Sim_GetRestoreTick();
ze_external void Sim_StartNewGame(char* mapName);

ze_external i32 Sim_ReserveDynamicIds(i32 count);
ze_external i32 Sim_ReserveStaticIds(i32 count);

ze_external ZEngine GetEngine();
ze_external zeHandle GetGameScene();

// general entities
ze_external Ent2d* Sim_GetFreeEntity(i32 id, i32 type);
ze_external Ent2d* Sim_GetEntById(i32 id);
ze_external EntityType* Sim_GetEntityType(i32 typeId);
ze_external void Sim_RestoreEntity(EntStateHeader* header);
ze_external void Sim_RemoveEntityBase(Ent2d* ent);
ze_external void Sim_RemoveEntity(Ent2d* ent);
ze_external void Sim_SyncDrawObjToPhysicsObj(zeHandle drawId, zeHandle bodyId);

// specific entities
ze_external Ent2d* Sim_FindPlayer();
ze_external i32 Sim_GetPlayerStatus();
ze_external void Sim_PlayerDied(Ent2d* player);

// spawning
ze_external void Sim_SpawnDebris(Vec2 pos, Vec2 velocity, f32 spin);
ze_external void Sim_SpawnPlayer(Vec2 pos);
ze_external void Sim_SpawnProjectile(
	Vec2 pos, f32 degrees, i32 teamId, i32 templateId);
ze_external void Sim_SpawnEnemyGrunt(Vec2 pos, i32 sourceId);
ze_external void Sim_SpawnGfx(Vec2 pos, i32 subType);
ze_external void Sim_SpawnSpawner(Vec2 pos);

// interactions
ze_external EntHitResponse Ent_Hit(
	Ent2d* attacker, Ent2d* victim, DamageHit* hit);
ze_external void Ent_MessageOnDeathById(i32 targetId, i32 victimId);

// registration
ze_external void EntNull_Register(EntityType* type);
ze_external void EntDebris_Register(EntityType* type);
ze_external void EntPlayer_Register(EntityType* type);
ze_external void EntGrunt_Register(EntityType* type);
ze_external void EntPointProjectile_Register(EntityType* type);
ze_external void EntGfxSprite_Register(EntityType* type);
ze_external void EntSpawner_Register(EntityType* type);

ze_external void EntPlayer_SetInput(RNGTickInfo info);

ze_external void Sim_DebugScanFrameData(i32 firstFrame, i32 maxFrames);

// editor
ze_external void Ed_Init(ZEngine engine);
ze_external void Ed_Enable();
ze_external void Ed_Disable();
ze_external void Ed_Tick(ZEFrameTimeInfo timing);

// application
ze_external void Menu_Init(ZEngine engine);
ze_external void Menu_Show(int pageOverride);
ze_external void Menu_Hide();
ze_external void Menu_Tick(ZEFrameTimeInfo timing);

#endif // RNG_INTERNAL_H

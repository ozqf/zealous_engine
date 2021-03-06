#pragma once

#include "../../headers/common/ze_common.h"
#include "../../headers/common/ze_math_types.h"

#include "../../headers/zr_asset_db.h"
#include "../../headers/voxel_world.h"
//#include "../physics/physics.h"

//#define SIM_USE_PHYSICS_ENGINE

#define SIM_SAVE_MAGIC_STRING "SIM_SAV"
#define SIM_SAVE_SENTINEL ZE_SENTINEL_B

#define SIM_GAME_STATE_WARMUP 0
#define SIM_GAME_STATE_GAMEPLAY 1
#define SIM_GAME_STATE_GAME_OVER 2

#define SIM_GAME_RULES_NONE 0
#define SIM_GAME_RULES_SURVIVAL 1

#define SIM_PLAYER_STATE_NONE 0
#define SIM_PLAYER_STATE_OBSERVING 1
#define SIM_PLAYER_STATE_IN_GAME 2
#define SIM_PLAYER_STATE_DEAD 3

#define SIM_LAYER_WORLD (1 << 0)

#define SIM_ENT_TEAM_FREELANCE 0
#define SIM_ENT_TEAM_PLAYER 1
#define SIM_ENT_TEAM_ENEMY 2
#define SIM_ENT_TEAM_NON_COMBATANT 3


// TODO: This is set for quantisating individual axes,
// but will need to be applied as vector magnitude to work properly
#define SIM_MAX_AXIS_SPEED 127

//#define SIM_QUANTISE_SYNC
#define SIM_QUANTISE_SPAWNS

#define SIM_NET_MIN_PRIORITY 1
#define SIM_NET_MAX_PRIORITY 6

#define SIM_DEFAULT_SPAWN_DELAY 1.5

#define SIM_ENT_MOVE_TYPE_NONE 0
#define SIM_ENT_MOVE_TYPE_FLOAT 1
#define SIM_ENT_MOVE_TYPE_WALK 1

#define SIM_ENT_STATUS_FREE 0
#define SIM_ENT_STATUS_RESERVED 1
#define SIM_ENT_STATUS_IN_USE 2
#define SIM_ENT_STATUS_CULL 3

#define SIM_ENT_NULL_SERIAL 0

#define SIM_DEATH_STYLE_NONE 0
#define SIM_DEATH_STYLE_TIMEOUT 1
#define SIM_DEATH_STYLE_SHOT 2

// Spawn functions
typedef u8 simFactoryType;

#define SIM_FACTORY_TYPE_NONE 0
#define SIM_FACTORY_TYPE_WORLD 1
#define SIM_FACTORY_TYPE_ACTOR 2
#define SIM_FACTORY_TYPE_PROJECTILE_BASE 3
#define SIM_FACTORY_TYPE_SPAWNER 4
#define SIM_FACTORY_TYPE_WANDERER 5
#define SIM_FACTORY_TYPE_LINE_TRACE 6
#define SIM_FACTORY_TYPE_PROJ_PREDICTION 7
#define SIM_FACTORY_TYPE_PROJ_PLAYER 8
#define SIM_FACTORY_TYPE_BOUNCER 9
#define SIM_FACTORY_TYPE_SEEKER 10
#define SIM_FACTORY_TYPE_RUBBLE 11
#define SIM_FACTORY_TYPE_DART 12
#define SIM_FACTORY_TYPE_EXPLOSION 13
#define SIM_FACTORY_TYPE_BOT 14
#define SIM_FACTORY_TYPE_GRUNT 15
#define SIM_FACTORY_TYPE_BRUTE 16
#define SIM_FACTORY_TYPE_CHARGER 17
#define SIM_FACTORY_TYPE_BULLET_IMPACT 18
#define SIM_FACTORY_TYPE_TARGET_POINT 19
#define SIM_FACTORY_TYPE_SEEKER_FLYING 20
#define SIM_FACTORY_TYPE_PROP_BILLBOARD 21
#define SIM_FACTORY_TYPE_PROP_MESH 22
#define SIM_FACTORY_TYPE_POINT_LIGHT 23
#define SIM_FACTORY_TYPE_DIRECT_LIGHT 24

// Update functions
#define SIM_TICK_TYPE_NONE 0
#define SIM_TICK_TYPE_WORLD 1
#define SIM_TICK_TYPE_ACTOR 2
#define SIM_TICK_TYPE_PROJECTILE 3
#define SIM_TICK_TYPE_SPAWNER 4
#define SIM_TICK_TYPE_WANDERER 5
#define SIM_TICK_TYPE_LINE_TRACE 6
#define SIM_TICK_TYPE_BOUNCER 7
#define SIM_TICK_TYPE_SEEKER 9
#define SIM_TICK_TYPE_DART 10
#define SIM_TICK_TYPE_EXPLOSION 11
#define SIM_TICK_TYPE_SPAWN 12
#define SIM_TICK_TYPE_BOT 13
#define SIM_TICK_TYPE_GRUNT 14
#define SIM_TICK_TYPE_TARGET_POINT 15
#define SIM_TICK_TYPE_SEEKER_FLYING 16
#define SIM_TICK_TYPE_PROJECTILE_VOLUME 17
#define SIM_TICK_TYPE_STUN 18
#define SIM_TICK_TYPE_PROJECTILE_ATTACK 19

#define SIM_MAX_ENT_UPDATERS 32

#define SIM_TICK_SUBMODE_NONE 0

#define SIM_ITEM_EVENT_TYPE_NONE 0
#define SIM_ITEM_EVENT_TYPE_PROJECTILE 1

// Spawn pattern types.
#define SIM_PATTERN_NONE 0
#define SIM_PATTERN_FLAT_CONE 1
#define SIM_PATTERN_3D_CONE 2
#define SIM_PATTERN_FLAT_RADIAL 3
#define SIM_PATTERN_FLAT_SCATTER 4
#define SIM_PATTERN_LINE 5
#define SIM_PATTERN_CIRCLE 6
#define SIM_PATTERN_3D_SCATTER 7

// Sim Entity flags
#define SIM_ENT_FLAG_OUT_OF_PLAY (1 << 0)
#define SIM_ENT_FLAG_SHOOTABLE (1 << 1)
#define SIM_ENT_FLAG_POSITION_SYNC (1 << 2)
#define SIM_ENT_FLAG_MOVE_AVOID (1 << 3)
#define SIM_ENT_FLAG_TARGET_SEEKING (1 << 4)
#define SIM_ENT_FLAG_USE_OVERRIDE_SCALE (1 << 5)
#define SIM_ENT_FLAG_INVULNERABLE (1 << 6)
#define SIM_ENT_FLAG_IGNORE_STUN (1 << 7)

#define SIM_DEATH_GFX_NONE 0
#define SIM_DEATH_GFX_BULLET_IMPACT 1
#define SIM_DEATH_GFX_GIB 2

#include "sim_types.h"
#include "sim_command_types.h"
#include "sim_assets.h"

#define SIM_DEFINE_ENT_UPDATE_FN(updateFuncName) internal void \
    updateFuncName##(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)

typedef void (*SimEntUpdate)(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer);

// Scene management
extern "C" void     Sim_Init(ZE_FatalErrorFunction fatalFunc, char* label, SimScene* sim, SimEntity* entityMemory, i32 maxEntities);
extern "C" void 	Sim_Reset(SimScene* sim);
// Load static/local geometry/entities. Dynamic stuff handled by server!
extern "C" i32      Sim_LoadMapFile(SimScene* sim, const char* mapName, i32 bLocalOnly);
extern "C" i32      Sim_CalcEntityArrayBytes(i32 capacity);
extern "C" i32		Sim_GetFrameNumber(SimScene* sim);
extern "C" timeFloat Sim_GetFrameInterval(SimScene* sim);
extern "C" frameInt Sim_CalcThinkTick(SimScene* sim, timeFloat secondsToThink);
extern "C" void     Sim_DumpCommandBuffer(SimScene* sim, ZEBuffer* buf);

extern "C" i32      Sim_Tick(
                        SimScene* sim,
                        ZEBuffer* input,
                        ZEBuffer* output,
                        ZEBuffer* soundOutput,
                        timeFloat delta);
// Players
extern "C" i32          SimPlyr_ReserveId(SimScene* sim);
extern "C" SimPlayer*   SimPlyr_Get(SimScene* sim, i32 playerId);

// Entity list functions
extern "C" SimEntity* Sim_GetEntityBySerial(SimScene* sim, i32 serial);
extern "C" SimEntity* Sim_GetEntityByIndex(SimScene* sim, SimEntIndex index);
extern "C" i32      Sim_ScanForSerialRange(SimScene* sim, i32 firstSerial, i32 numSerials);
extern "C" i32      Sim_ReserveEntitySerial(SimScene* sim, i32 isLocal);
extern "C" i32      Sim_ReserveEntitySerials(
                        SimScene* scene, i32 isLocal, i32 count);
extern "C" SimEntity* Sim_RestoreEntity(SimScene* sim, SimEvent_Spawn* def);
//extern "C" i32      Sim_RemoveEntity(SimScene* sim, i32 serialNumber);
extern "C" i32      Sim_SetActorInput(SimScene* sim, SimActorInput* input, i32 entitySerial);
extern "C" i32      Sim_ExecuteBulkSpawn(
                        SimScene* sim,
                        SimEvent_BulkSpawn* def,
						i32 fastForwardTicks,
                        i32* spawnedEntityFlags,
                        f32* spawnedEntityPriority);
extern "C" void Sim_PrepareSpawnData(
                        SimScene* sim, SimEvent_Spawn* data,
                        i32 bIsLocal, u8 factoryType,
                        Vec3 pos);

// Entity behaviour
extern "C" void Sim_SimpleMove(SimScene* sim, SimEntity* ent, SimEntMovement* mover, timeFloat delta);
extern "C" i32      Sim_InBounds(SimEntity* ent, Vec3* min, Vec3* max);
extern "C" void     Sim_BoundaryBounce(SimEntity* ent, Vec3* min, Vec3* max);
extern "C" void     Sim_BoundaryStop(SimEntity* ent, Vec3* min, Vec3* max);
extern "C" i32      Sim_GroundCheck(SimScene* sim, SimEntity* ent);

extern "C" SimInventoryItem* SVI_GetItem(i32 index);

extern "C" void     Sim_TickDebugCamera(Transform* t, SimActorInput input, f32 moveSpeed, timeFloat delta);

// Searching/Querying
extern "C" i32        Sim_IsEntInPlay(SimEntity* ent);
extern "C" SimEntity* Sim_FindTargetForEnt(SimScene* sim, SimEntity* subject);
extern "C" i32        Sim_IsEntTargetable(SimEntity* ent);
extern "C" i32      Sim_FindClosestRayhit(SimRaycastResult* results, i32 numResults);

extern "C" i32      Sim_FindByRaycast(
                        SimScene* sim,
                        Vec3 origin,
                        Vec3 dest,
                        Vec3 objSizeInflate,
                        i32 ignoreSerial,
                        SimRaycastResult* results,
                        i32 maxResults);

extern "C" i32 Sim_FindFirstByRay(
                        SimScene* sim,
                        Vec3 origin,
                        Vec3 dest,
                        SimRaycastResult* result,
                        i32 ignoreSerial);

extern "C" i32      Sim_FindByAABB(
                        SimScene* sim,
                        Vec3 boundsMin,
                        Vec3 boundsMax,
                        i32 ignoreSerial,
                        SimEntity** results,
                        i32 maxResults,
	                    i32 replicatedOnly);

extern "C" SimAvoidInfo Sim_BuildAvoidVector(
                        SimScene* sim,
                        SimEntity* mover,
                        f32 boundingBoxScale);

#pragma once

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_math_types.h"

//#include "../physics/physics.h"

//#define SIM_USE_PHYSICS_ENGINE

#define SIM_LAYER_WORLD (1 << 0)

// TODO: This is set for quantisating individual axes,
// but will need to be applied as vector magnitude to work properly
#define SIM_MAX_AXIS_SPEED 127

//#define SIM_QUANTISE_SYNC
#define SIM_QUANTISE_SPAWNS

#define SIM_NET_MIN_PRIORITY 1
#define SIM_NET_MAX_PRIORITY 6

#define SIM_DEFAULT_SPAWN_DELAY 1.5

#define SIM_ENT_STATUS_FREE 0
#define SIM_ENT_STATUS_RESERVED 1
#define SIM_ENT_STATUS_IN_USE 2

#define SIM_ENT_NULL_SERIAL 0

// Render prefabs
#define SIM_PREFAB_NONE 0
#define SIM_PREFAB_PLAYER 1
#define SIM_PREFAB_PLAYER_PROJECTILE 2
#define SIM_PREFAB_BOT 3
#define SIM_PREFAB_WALL 4
#define SIM_PREFAB_ENEMY 5
#define SIM_PREFAB_ENEMY_PROJECTILE 6
#define SIM_PREFAB_ITEM 7
#define SIM_PREFAB_EXPLOSION 8

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

#define SIM_DEATH_GFX_NONE 0
#define SIM_DEATH_GFX_EXPLOSION 1

#include "sim_types.h"

// Scene management
extern "C" void     Sim_Init(
                        char* label,
                        SimScene* sim,
                        SimEntity* entityMemory,
                        i32 maxEntities);
extern "C" void 	Sim_Reset(SimScene* sim);
extern "C" i32 	    Sim_LoadScene(SimScene* sim, i32 index);
extern "C" i32      Sim_CalcEntityArrayBytes(i32 capacity);
extern "C" i32		Sim_GetFrameNumber(SimScene* sim);

// Entity list functions
extern "C" SimEntity* Sim_GetEntityBySerial(SimScene* sim, i32 serial);
extern "C" SimEntity* Sim_GetEntityByIndex(SimScene* sim, SimEntIndex index);
extern "C" i32      Sim_ScanForSerialRange(SimScene* sim, i32 firstSerial, i32 numSerials);
extern "C" i32      Sim_ReserveEntitySerial(SimScene* sim, i32 isLocal);
extern "C" i32      Sim_ReserveEntitySerials(
                        SimScene* scene, i32 isLocal, i32 count);
extern "C" SimEntity* Sim_RestoreEntity(SimScene* sim, SimEntSpawnData* def);
extern "C" i32      Sim_RemoveEntity(SimScene* sim, i32 serialNumber);
extern "C" i32      Sim_SetActorInput(
    SimScene* sim, SimActorInput* input, i32 entitySerial);
extern "C" i32      Sim_ExecuteBulkSpawn(
                        SimScene* sim,
                        SimBulkSpawnEvent* def,
						i32 fastForwardTicks,
                        i32* spawnedEntityFlags,
                        f32* spawnedEntityPriority);

// Entity behaviour
extern "C" void     Sim_SimpleMove(SimEntity* ent, timeFloat deltaTime);
extern "C" i32      Sim_InBounds(SimEntity* ent, Vec3* min, Vec3* max);
extern "C" void     Sim_BoundaryBounce(SimEntity* ent, Vec3* min, Vec3* max);
extern "C" void     Sim_BoundaryStop(SimEntity* ent, Vec3* min, Vec3* max);

// Entity Frame Updates
extern "C" i32      SimEnt_TickSpawnAnimation(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime);

extern "C" void     SimEnt_TickWanderer(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer);

extern "C" void     SimEnt_TickSeeker(
                        SimScene* sim,
                        SimEntity* ent,
                        timeFloat deltaTime,
                        i32 bIsServer);

extern "C" void     SimEnt_TickSeekerFlying(
                        SimScene* sim,
                        SimEntity* ent,
                        timeFloat deltaTime,
                        i32 bIsServer);
                        
extern "C" void     SimEnt_StepActorMovement(
                        SimScene* sim,
                        SimEntity* ent,
                        SimActorInput* input,
                        timeFloat deltaTime);

// Searching/Querying
extern "C" i32        Sim_IsEntInPlay(SimEntity* ent);
extern "C" SimEntity* Sim_FindTargetForEnt(SimScene* sim, SimEntity* subject);
extern "C" i32        Sim_IsEntTargetable(SimEntity* ent);

extern "C" i32 Sim_FindClosestRayhit(SimRaycastResult* results, i32 numResults);

extern "C"
i32 Sim_FindByRaycast(
    SimScene* sim,
    Vec3 origin,
    Vec3 dest,
    i32 ignoreSerial,
    SimRaycastResult* results,
    i32 maxResults);

extern "C"
inline i32 Sim_FindByAABB(
    SimScene* sim,
    Vec3 boundsMin,
    Vec3 boundsMax,
    i32 ignoreSerial,
    SimEntity** results,
    i32 maxResults,
	i32 replicatedOnly);

extern "C"
inline SimAvoidInfo Sim_BuildAvoidVector(
    SimScene* sim,
    SimEntity* mover,
    f32 boundingBoxScale);

#pragma once

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_bitpack.h"
#include "../zqf_renderer.h"
#include "../physics/physics.h"

#define ACTOR_INPUT_MOVE_FORWARD (1 << 0)
#define ACTOR_INPUT_MOVE_BACKWARD (1 << 1)
#define ACTOR_INPUT_MOVE_LEFT (1 << 2)
#define ACTOR_INPUT_MOVE_RIGHT (1 << 3)
#define ACTOR_INPUT_MOVE_UP (1 << 4)
#define ACTOR_INPUT_MOVE_DOWN (1 << 5)
#define ACTOR_INPUT_ATTACK (1 << 6)
#define ACTOR_INPUT_ATTACK2 (1 << 7)
#define ACTOR_INPUT_MOVE_SPECIAL1 (1 << 8)

#define ACTOR_INPUT_SHOOT_LEFT (1 << 27)
#define ACTOR_INPUT_SHOOT_RIGHT (1 << 28)
#define ACTOR_INPUT_SHOOT_UP (1 << 29)
#define ACTOR_INPUT_SHOOT_DOWN (1 << 30)
struct SimActorInput
{
    u32 buttons;
    Vec3 degrees;
};

struct SimAvoidInfo
{
    Vec3 dir;
    i32 numNeighbours;
};

#pragma pack(push, 1)
union SimEntIndex
{
    struct
    {
        u16 iteration;
        u16 index;
    };
    u16 arr[2];
    u32 value;
};
#pragma pack(pop)

// Note: A serial of 0 should be considered null
// and that this Entity Id relates to no entity, even if
// the slot is set.
struct SimEntId
{
    // Location in local entity memory
    // Will differ between client and server!
    SimEntIndex slot;
    // unique, replicated Id.
    i32 serial;
};

struct SimEntDisplay
{
    i32 prefabIndex;
    Vec3 scale;
    Colour colourA;
    Colour colourB;
};

struct SimEntity
{
    i32 status;
    i32 isLocal;
    SimEntId id;

    simFactoryType factoryType;
    i32 tickType;
    i32 coreTickType;

    ZShapeDef shape;

    struct 
    {
        // TODO: For the client only the serial of targetid is safe.
        // Slot is not!!
        SimEntId targetId;  // current enemy
        SimEntId parentId;  // Who spawned this entity
        simFactoryType childFactoryType;
        i32 childSpawnCount;
        i32 liveChildren;
        i32 maxLiveChildren;
        i32 totalChildren;
    } relationships;
    
    // An abritrary position this entity is moving toward
    Vec3 destination;

    struct
    {
        i32 lastThink;
        i32 nextThink;

        i32 birthTick;
        // This entity was spawned in the past this many ticks ago
        // and needs to catch up
	    i32 fastForwardTicks;
    } timing;

    f32 attackTick;
    f32 attackTime;

    SimEntDisplay display;
    u8 deathType;

    u32 flags;
    u32 localFlags;

    // physical
    struct
    {
        Transform t;
        Vec3 previousPos;
        Vec3 error;
        f32 errorRate;
        f32 speed;
        Vec3 velocity;
        f32 pitch;
        f32 yaw;
        Vec3 halfSize;
    } body;

    struct
    {
        i16 health;
    } life;
    
	SimActorInput input;

    f32 priority;
    f32 basePriority;
    struct
    {
        i32 lastSync;
    } clientOnly;
    
};

struct SimEntSpawnData
{
    i32 serial;
    i32 parentSerial;
    i32 fastForwardTicks;

    i32 birthTick;
    
    i32 isLocal;
    // Spawner info... could be used for attacks too...?
    simFactoryType factoryType;
    simFactoryType childFactoryType;
    u8 numChildren;

    Vec3 pos;
    Vec3 scale;
    Vec3 velocity;
    Vec3 destination;
};

struct SimSpawnPatternItem
{
	i32 entSerial;
	Vec3 pos;
	Vec3 forward;
};

struct SimSpawnPatternDef
{
	i32 patternId;
	i32 numItems;
	f32 radius;
    f32 arc;
};

struct SimProjectileType
{
    // Characterics of each projectile
    f32 speed;
    f32 lifeTime;
    f32 arcDegrees;

    ColourU32 colour;
    Vec3 scale;

    // Characterics of how this projectile is spawned
    SimSpawnPatternDef patternDef;
};

struct SimSpawnBase
{
    Vec3 pos;
    Vec3 forward;
    i32 firstSerial;
    i32 sourceSerial;
    i32 tick;
    u8 seedIndex;
};

struct SimBulkSpawnEvent
{
    SimSpawnBase base;
    SimSpawnPatternDef patternDef;
    u8 factoryType;
};

internal void Sim_SetBulkSpawn(
    SimBulkSpawnEvent* ev,
    i32 firstSerial,
    i32 sourceSerial,
    Vec3 pos,
    Vec3 forward,
    i32 tick,
    u8 factoryType,
    u8 patternId,
    u8 numItems,
    u8 seedIndex,
    f32 radius,
    f32 arc)
{
    ev->factoryType = factoryType;
    ev->base.firstSerial = firstSerial;
    ev->base.sourceSerial = sourceSerial;
    ev->base.pos = pos;
    ev->base.forward = forward;
    ev->base.tick = tick;
    ev->base.seedIndex = seedIndex;
    #ifdef SIM_QUANTISE_SYNC
    ev->base.forward = ZE_UnpackVec3Normal(
        ZE_PackVec3NormalToI32(forward.parts));
    #endif
    ev->factoryType = factoryType;
    ev->patternDef.patternId = patternId;
    ev->patternDef.numItems = numItems;
    ev->patternDef.radius = radius;
    ev->patternDef.arc = arc;
}

#if 0
// For block allocated entity storage
struct SimEntBlock
{
    i32 index;
    SimEntity* ents;
    i32 capacity;
};
#endif
struct SimScene
{
    SimEntity* ents;
    i32 maxEnts;

    WorldHandle* world;

    // sequential, unrelated to blocks
    i32 remoteEntitySequence;
    i32 localEntitySequence;
    i32 cmdSequence;
    i32 tick;
	
	i64 timeInAABBSearch;

    // for client. server has remote sequence anyway
    i32 highestAssignedSequence;

    Vec3 boundaryMin;
    Vec3 boundaryMax;

    QuantiseSet quantise;
};

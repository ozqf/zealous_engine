#ifndef SIM_COMMAND_TYPES_H
#define SIM_COMMAND_TYPES_H

#include "sim.h"

#define SIM_CMD_TYPE_BULK_SPAWN 255

#if 0
struct SimCommand
{
	ZECommand header;
	i32 tick;
};
#endif

// Configurable data for spawning an entity.
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
    u8 patternType;
    u8 numChildren;

    Vec3 pos;
    Vec3 scale;
    Vec3 velocity;
    Vec3 destination;
    f32 pitchDegrees;
    f32 yawDegrees; 

    union
    {
        struct
        {
            Vec3 colour;
            f32 multiplier;
            f32 range;
        } pointLight;
    };
};

struct SimSpawnBase
{
    i32 firstSerial;
    i32 sourceSerial;
    i32 tick;
    u8 seedIndex;
    Transform xForm;
};

struct SimBulkSpawnEvent
{
	ZECommand header;
    SimSpawnBase base;
    SimSpawnPatternDef patternDef;
    u8 factoryType;
};

internal void Sim_SetBulkSpawn(
    SimBulkSpawnEvent* ev,
    i32 firstSerial,
    i32 sourceSerial,
    Transform xForm,
    i32 tick,
    u8 factoryType,
    u8 patternId,
    u8 numItems,
    u8 seedIndex,
    f32 radius,
    f32 arc)
{
	ZCmd_Prepare(&ev->header, SIM_CMD_TYPE_BULK_SPAWN, sizeof(SimBulkSpawnEvent));
	
    ev->factoryType = factoryType;
    ev->base.firstSerial = firstSerial;
    ev->base.sourceSerial = sourceSerial;
    ev->base.xForm = xForm;
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

#endif // SIM_COMMAND_TYPES_H
#include "sim_internal.h"

internal void Sim_AddWorldVolume(SimScene* sim, Vec3 pos, Vec3 size)
{
    SimEvent_Spawn def = {};
    def.serial = Sim_ReserveEntitySerial(sim, 1);
    def.isLocal = 1;
	def.factoryType = SIM_FACTORY_TYPE_WORLD;
    def.pos = pos;
    def.scale = size;
    Sim_RestoreEntity(sim, &def);
}

internal void Sim_AddPointLight(SimScene* sim, Vec3 pos, Vec3 colour, f32 multiplier, f32 range)
{
    SimEvent_Spawn def = {};
    def.serial = Sim_ReserveEntitySerial(sim, 1);
    def.isLocal = 1;
	def.factoryType = SIM_FACTORY_TYPE_POINT_LIGHT;
    def.pos = pos;
    def.pointLight.colour = colour;
    def.pointLight.multiplier = multiplier;
    def.pointLight.range = range;
    Sim_RestoreEntity(sim, &def);
}

internal void Sim_AddDirectLight(
    SimScene* sim, Vec3 pos, Vec3 colour, f32 multiplier, f32 range,
    f32 pitchDegrees, f32 yawDegrees)
{
    SimEvent_Spawn def = {};
    def.serial = Sim_ReserveEntitySerial(sim, 1);
    def.isLocal = 1;
	def.factoryType = SIM_FACTORY_TYPE_DIRECT_LIGHT;
    def.pos = pos;
    def.pointLight.colour = colour;
    def.pointLight.multiplier = multiplier;
    def.pointLight.range = range;
    def.pitchDegrees = pitchDegrees;
    def.yawDegrees = yawDegrees;
    Sim_RestoreEntity(sim, &def);
}

internal void Sim_AddTestProp(SimScene* sim, Vec3 pos, i32 billboard)
{
    SimEvent_Spawn def = {};
    def.serial = Sim_ReserveEntitySerial(sim, YES);
    def.isLocal = 1;
	if (billboard)
	{
		def.factoryType = SIM_FACTORY_TYPE_PROP_BILLBOARD;
	}
	else
	{
		def.factoryType = SIM_FACTORY_TYPE_PROP_MESH;
	}
	
	
    def.pos = pos;
    Sim_RestoreEntity(sim, &def);
}

internal void Sim_AddLightTower(SimScene* sim, Vec3 basePos, f32 height)
{
    i32 step = 2;
    i32 numStacks = (i32)(height / (f32)step);
    //printf("Num light stacks: %d\n", numStacks);
    f32 lightMultiplier = 1.f;
    f32 lightRange = 1.5f;
    f32 posOffset = 1;
    for (i32 i = 0; i < numStacks; ++i)
    {
        //printf("\tBase y: %.3f\n", basePos.y);
        Vec3 pos = basePos;
        pos.x += posOffset;
        Sim_AddPointLight(sim, pos, { 1, 0, 0}, lightMultiplier, lightRange);
        pos = basePos;
        pos.x -= posOffset;
        Sim_AddPointLight(sim, pos, { 1, 0, 0}, lightMultiplier, lightRange);

        pos = basePos;
        pos.z += posOffset;
        Sim_AddPointLight(sim, pos, { 1, 0, 0}, lightMultiplier, lightRange);
        pos = basePos;
        pos.z -= posOffset;
        Sim_AddPointLight(sim, pos, { 1, 0, 0}, lightMultiplier, lightRange);

        basePos.y += step;
    }
}

internal ErrorCode Sim_LoadStaticScene(SimScene* sim, i32 index)
{
    printf("SIM - load static scene %d\n", index);
    f32 halfX = 25;
    f32 halfY = 25;
    f32 halfZ = 25;
    const i32 largestHalfAxis = 35 + 10;

    // Floor
    Sim_AddWorldVolume(sim, { 0, -1, 0 }, { halfX * 2, 1, halfY * 2});

    // sunlights
    #if 0 // faint

    Sim_AddDirectLight(sim, { 0, 5, 0 }, { 1, 1, 0 }, 0.5f, 999, -45, 45);
    Sim_AddDirectLight(sim, { 0, 5, 0 }, { 0, 0, 1 }, 0.2f, 999, -45, 225);
    #endif
    #if 0 // bright

    Sim_AddDirectLight(sim, { 0, 5, 0 }, { 1, 1, 0 }, 2.f, 999, -45, 45);
    Sim_AddDirectLight(sim, { 0, 5, 0 }, { 0, 0, 1 }, 1.f, 999, -45, 225);
    #endif

    // flood lights
    f32 floodLightY = 10;
    f32 floodLightMul = 2;
    f32 floodLightRange = 25;// 18;
	Sim_AddPointLight(sim, { 0, floodLightY, 0 }, { 1, 1, 1 }, 1, 18);
    Sim_AddPointLight(sim, { 15, floodLightY, 15 }, { 1, 0, 0 }, floodLightMul, floodLightRange);
    Sim_AddPointLight(sim, { 15, floodLightY, -15 }, { 1, 1, 0 }, floodLightMul, floodLightRange);
	Sim_AddPointLight(sim, { -15, floodLightY, -15 }, { 0, 1, 0 }, floodLightMul, floodLightRange);
	Sim_AddPointLight(sim, { -15, floodLightY, 15 }, { 0, 1, 1 }, floodLightMul, floodLightRange);

    // pillars
    f32 pillarY = 4;
	// Inner
    Sim_AddWorldVolume(sim, { 0, pillarY, -10 }, { 1, 10, 1 });
    // Sim_AddLightTower(sim, { 0, 0, -10 }, 10);
    Sim_AddWorldVolume(sim, { 0, pillarY, 10 }, { 1, 10, 1 });
    // Sim_AddLightTower(sim, { 0, 0, 10 }, 10);

    Sim_AddWorldVolume(sim, { -10, pillarY, 0 }, { 1, 10, 1 });
    // Sim_AddLightTower(sim, { -10, 0, 0 }, 10);
    Sim_AddWorldVolume(sim, { 10, pillarY, 0 }, { 1, 10, 1 });
    // Sim_AddLightTower(sim, { 10, 0, 0 }, 10);

	// outer
	Sim_AddWorldVolume(sim, { -halfX, pillarY, -halfZ }, { 1, 10, 1 });
	Sim_AddWorldVolume(sim, { halfX, pillarY, -halfZ }, { 1, 10, 1 });
	Sim_AddWorldVolume(sim, { -halfX, pillarY, halfZ }, { 1, 10, 1 });
	Sim_AddWorldVolume(sim, { halfX, pillarY, halfZ }, { 1, 10, 1 });
    
    APP_PRINT(128, "SIM spawn props\n")
    // static sprites
    Sim_AddTestProp(sim, { 15, 2, 15 }, 1);
    Sim_AddTestProp(sim, { 15, 2, -15 }, 0);
    Sim_AddTestProp(sim, { -15, 2, 15 }, 1);
    Sim_AddTestProp(sim, { -15, 2, -15 }, 0);

    halfX -= 1;
    halfY -= 1;
    halfZ -= 1;

    sim->boundaryMin = { -halfX, -halfY, -halfZ };
    sim->boundaryMax = { halfX, halfY, halfZ };

    // Configure quantisation tables based on arena size
    // TODO: largest axis must be passed in here, auto detect this instead!
    COM_QuantiseInit(&sim->quantise.pos, largestHalfAxis, 16);
    COM_QuantiseInit(&sim->quantise.vel, SIM_MAX_AXIS_SPEED, 16);
    // TODO: Configure this more precisely for radians
    COM_QuantiseInit(&sim->quantise.rot, 7, 16);

    // Setup player spawn position
    sim->playerStartPos = { -6, 15, 6 };

    // Setup default camera position
    Transform_SetToIdentity(&sim->observePos);
    sim->observePos.pos.z = 10;
    sim->observePos.pos.y += 34;
    Transform_SetRotation(&sim->observePos, -(80.0f    * DEG2RAD), 0, 0);
    

    return ZE_ERROR_NONE;
}

internal void SV_AddSpawner(
    SimScene* sim, Vec3 pos, simFactoryType factoryType, u8 spawnCount)
{
    SimEvent_Spawn data = {};
    data.numChildren = spawnCount;
    Sim_PrepareSpawnData(sim, &data, 1, SIM_FACTORY_TYPE_SPAWNER, pos);
    data.childFactoryType = factoryType;
    data.numChildren = spawnCount;
    data.patternType = SIM_PATTERN_FLAT_SCATTER;
    //data.patternType = SIM_PATTERN_3D_SCATTER;
    Sim_RestoreEntity(sim, &data);
}

internal ErrorCode Sim_LoadDynamicEntities(SimScene* sim, i32 index)
{
    printf("SIM - load dynamic scene %d\n", index);
	//SV_AddSpawner(sim, { 0, 10, 0 }, SIM_FACTORY_TYPE_WANDERER, 1);
    //SV_AddSpawner(sim, { 0, 10, 0 }, SIM_FACTORY_TYPE_BOUNCER, 1);
    
    switch (index)
    {
        case 1:
        SV_AddSpawner(sim, { 10, 10, 10 }, SIM_FACTORY_TYPE_SEEKER, 10);
        SV_AddSpawner(sim, { -10, 10, -10 }, SIM_FACTORY_TYPE_SEEKER, 10);
        SV_AddSpawner(sim, { 10, 10, -10 }, SIM_FACTORY_TYPE_BOUNCER, 10);
        SV_AddSpawner(sim, { -10, 10, 10 }, SIM_FACTORY_TYPE_BOUNCER, 10);
        break;
        case 2:
        SV_AddSpawner(sim, { 0, 10, 0 }, SIM_FACTORY_TYPE_SEEKER, 1);
        break;
        case 3:
        SV_AddSpawner(sim, { 0, 10, 0 }, SIM_FACTORY_TYPE_WANDERER, 10);
        break;
        default:
        APP_PRINT(128, "SIM - no dynamic entities\n");
        break;
    }
    return ZE_ERROR_NONE;
}

internal
i32 Sim_LoadEmbeddedScene(SimScene* sim, i32 index, i32 bLocalOnly)
{
	APP_PRINT(128, "SIM - load embedded scene\n")
    Sim_LoadStaticScene(sim, index);
    if (bLocalOnly == NO)
    {
        Sim_LoadDynamicEntities(sim, index);
    }
    APP_PRINT(128, "\tDONE\n")

	return ZE_ERROR_NONE;
}

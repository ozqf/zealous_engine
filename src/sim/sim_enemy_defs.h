#pragma once

#include "sim.h"


///////////////////////////////////////////////////////
// Enemies
///////////////////////////////////////////////////////
internal i32 Sim_InitWanderer(
    SimScene* scene, SimEntity* ent, SimEntSpawnData* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityStats(ent, 1, 1, 1);
    ent->tickType = SIM_TICK_TYPE_SPAWN;
    ent->coreTickType = SIM_TICK_TYPE_WANDERER;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = ent->timing.birthTick + App_CalcTickInterval(1.5f);
    Sim_SetEntityDisplay(ent,
        { 1, 0, 1, 1 },
        { 1, 0, 1, 1 },
        SIM_PREFAB_CUBE,
        SIM_DEATH_GFX_EXPLOSION);
    ent->flags = SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_OUT_OF_PLAY;
    ent->deathType = SIM_DEATH_GFX_EXPLOSION;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitRubble(
    SimScene* scene, SimEntity* ent, SimEntSpawnData* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityStats(ent, 4, 1, 1);
    ent->tickType = SIM_TICK_TYPE_SPAWN;
    ent->coreTickType = SIM_TICK_TYPE_NONE;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = ent->timing.birthTick + App_CalcTickInterval(1.5f);
    Sim_SetEntityDisplay(ent,
        { 0.7f, 0.7f, 1, 1 },
        { 0.7f, 0.7f, 1, 1 },
        SIM_PREFAB_CUBE,
        SIM_DEATH_GFX_EXPLOSION);
    ent->flags = SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_OUT_OF_PLAY;
    ent->deathType = SIM_DEATH_GFX_EXPLOSION;
    #ifdef SIM_USE_PHYSICS_ENGINE
    ent->shape.SetAsBox(def->pos, { 0.5f, 0.5f, 0.5f}, 0, SIM_LAYER_WORLD, SIM_LAYER_WORLD, 0);
    PhysCmd_CreateShape(scene->world, &ent->shape, ent->id.serial);
    #endif
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitBouncer(
    SimScene* scene, SimEntity* ent, SimEntSpawnData* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityStats(ent, 6.0f, 1, 1);
    ent->tickType = SIM_TICK_TYPE_SPAWN;
    ent->coreTickType = SIM_TICK_TYPE_BOUNCER;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = ent->timing.birthTick + App_CalcTickInterval(1.5f);
    Sim_SetEntityDisplay(ent,
        { 0.5f, 0.5f, 0.7f, 1 },
        { 0.5f, 0.5f, 0.7f, 1 },
        SIM_PREFAB_CUBE,
        SIM_DEATH_GFX_EXPLOSION);
    ent->flags = SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_OUT_OF_PLAY;
    ent->deathType = SIM_DEATH_GFX_EXPLOSION;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitDart(
    SimScene* scene, SimEntity* ent, SimEntSpawnData* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityStats(ent, 5.5f, 1, 1);
    ent->tickType = SIM_TICK_TYPE_SPAWN;
    ent->coreTickType = SIM_TICK_TYPE_DART;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = ent->timing.birthTick + App_CalcTickInterval(1.5f);
    Sim_SetEntityDisplay(ent,
        { 1, 0.7f, 0.3f, 1 },
        { 1, 0.7f, 0.3f, 1 },
        SIM_PREFAB_CUBE,
        SIM_DEATH_GFX_EXPLOSION);
    ent->flags = SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_MOVE_AVOID
        | SIM_ENT_FLAG_OUT_OF_PLAY;
    ent->deathType = SIM_DEATH_GFX_EXPLOSION;
    //printf("Spawn dart on tick %d birth %d last %d next %d\n",
    //    scene->tick,
    //    ent->birthTick,
    //    ent->timing.lastThink,
    //    ent->timing.nextThink);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitSeeker(
    SimScene* scene, SimEntity* ent, SimEntSpawnData* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityStats(ent, 4, 1, 1);
    ent->tickType = SIM_TICK_TYPE_SPAWN;
    ent->coreTickType = SIM_TICK_TYPE_SEEKER;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = ent->timing.birthTick + App_CalcTickInterval(1.5f);
    Sim_SetEntityDisplay(ent,
        { 0, 0.7f, 0.7f, 1 },
        { 0, 0.7f, 0.7f, 1 },
        SIM_PREFAB_CUBE,
        SIM_DEATH_GFX_EXPLOSION);
    ent->flags =
          SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_MOVE_AVOID
        | SIM_ENT_FLAG_OUT_OF_PLAY;
    ent->deathType = SIM_DEATH_GFX_EXPLOSION;
    ent->basePriority = 8;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitGrunt(
    SimScene* sim, SimEntity* ent, SimEntSpawnData* data)
{
    Sim_SetEntityBase(ent, data);
    Sim_SetEntityStats(ent, 3, 1, 1);
    ent->body.t.scale = { 1.5f, 1, 1.5f };
    ent->tickType = SIM_TICK_TYPE_SPAWN;
    ent->coreTickType = SIM_TICK_TYPE_GRUNT;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = ent->timing.birthTick + App_CalcTickInterval(1.5f);
    ent->flags =
          SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_MOVE_AVOID
        | SIM_ENT_FLAG_OUT_OF_PLAY;
    Sim_SetEntityDisplay(ent,
        { 1, 0.2f, 0.2f, 1 },
        { 1, 0.2f, 0.2f, 1 },
        SIM_PREFAB_CUBE,
        SIM_DEATH_GFX_EXPLOSION);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitBrute(
    SimScene* sim, SimEntity* ent, SimEntSpawnData* data)
{
    ZE_ASSERT(0, "Not implemented");
    Sim_SetEntityBase(ent, data);
    Sim_SetEntityStats(ent, 3, 1, 1);
    ent->body.t.scale = { 3.0f, 2.0f, 3.0f };
    ent->flags =
          SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_MOVE_AVOID
        | SIM_ENT_FLAG_OUT_OF_PLAY;
    Sim_SetEntityDisplay(ent,
        { 1, 0.2f, 0.2f, 1 },
        { 1, 0.2f, 0.2f, 1 },
        SIM_PREFAB_CUBE,
        SIM_DEATH_GFX_EXPLOSION);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitCharger(
    SimScene* sim, SimEntity* ent, SimEntSpawnData* data)
{
    ZE_ASSERT(0, "Not implemented");
    Sim_SetEntityBase(ent, data);
    Sim_SetEntityStats(ent, 3, 1, 1);
    ent->body.t.scale = { 2.0f, 2.0f, 2.0f };
    return ZE_ERROR_NONE;
}

#pragma once

#include "sim.h"


///////////////////////////////////////////////////////
// Enemies
///////////////////////////////////////////////////////

internal i32 Sim_InitRubble(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntLife(ent, NO, YES, 100);
    Sim_SetEntMoveType(ent, &ent->movement, 0, SIM_ENT_MOVE_TYPE_NONE, NO);

    Sim_SetEntityBody(ent, { 1, 1, 1 });
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 0.7f, 0.7f, 1, 1 },
        { 0.7f, 0.7f, 1, 1 },
        "Quad",
        "Enemy",
        SIM_DEATH_GFX_GIB);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    ent->display.data.model.billboard = 1;
    ent->deathType = SIM_DEATH_GFX_GIB;
    ent->think.tickType = SIM_TICK_TYPE_SPAWN;
    ent->think.coreTickType = SIM_TICK_TYPE_NONE;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = Sim_CalcThinkTick(scene, 1.5f);
    #ifdef SIM_USE_PHYSICS_ENGINE
    ent->shape.SetAsBox(def->pos, { 0.5f, 0.5f, 0.5f}, 0, SIM_LAYER_WORLD, SIM_LAYER_WORLD, 0);
    PhysCmd_CreateShape(scene->world, &ent->shape, ent->id.serial);
    #endif
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitWanderer(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntLife(ent, NO, YES, 100);
    Sim_SetEntMoveType(ent, &ent->movement, 3, SIM_ENT_MOVE_TYPE_WALK, NO);
    
    Sim_SetEntityBody(ent, { 1, 2, 1 });
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 0, 1, 1 },
        { 1, 0, 1, 1 },
        "Cube",
        "Enemy",
        SIM_DEATH_GFX_GIB);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    ent->movement.flags |= SIM_ENT_MOVE_BIT_BOUNDARY_BOUNCE;
    ent->deathType = SIM_DEATH_GFX_GIB;
    ent->think.tickType = SIM_TICK_TYPE_SPAWN;
    ent->think.coreTickType = SIM_TICK_TYPE_WANDERER;
    ent->timing.lastThink = ent->timing.birthTick; 
    ent->timing.nextThink = Sim_CalcThinkTick(scene, 1.5f);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitBouncer(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntLife(ent, NO, YES, 100);
    Sim_SetEntMoveType(ent, &ent->movement, 3, SIM_ENT_MOVE_TYPE_WALK, NO);
    ent->movement.flags |= SIM_ENT_MOVE_BIT_BOUNDARY_BOUNCE;

    Sim_SetEntityBody(ent, { 1, 2, 1});
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 0.5f, 0.5f, 0.7f, 1 },
        { 0.5f, 0.5f, 0.7f, 1 },
        "Cube",
        "Enemy",
        SIM_DEATH_GFX_GIB);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    ent->think.tickType = SIM_TICK_TYPE_SPAWN;
    ent->think.coreTickType = SIM_TICK_TYPE_BOUNCER;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = Sim_CalcThinkTick(scene, 1.5f);
    ent->deathType = SIM_DEATH_GFX_GIB;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitDart(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntLife(ent, NO, YES, 100);
    Sim_SetEntMoveType(ent, &ent->movement, 5.5f, SIM_ENT_MOVE_TYPE_WALK, NO);
    
    Sim_SetEntityBody(ent, { 1, 2, 1});
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 0.7f, 0.3f, 1 },
        { 1, 0.7f, 0.3f, 1 },
        "Cube",
        "Enemy",
        SIM_DEATH_GFX_GIB);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    ent->deathType = SIM_DEATH_GFX_GIB;
    ent->think.tickType = SIM_TICK_TYPE_SPAWN;
    ent->think.coreTickType = SIM_TICK_TYPE_DART;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = Sim_CalcThinkTick(scene, 1.5f);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitSeeker(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntLife(ent, NO, YES, 60);
    Sim_SetEntMoveType(ent, &ent->movement, 4, SIM_ENT_MOVE_TYPE_WALK, NO);
    
    // Sim_SetEntityBody(ent, { 1, 2, 1});
    Sim_SetEntityBody(ent, { 1, 1, 1});
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 0, 0.7f, 0.7f, 1 },
        { 0, 0.7f, 0.7f, 1 },
        SIM_MODEL_SEEKER,
        "Enemy",
        SIM_DEATH_GFX_GIB);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    ent->think.tickType = SIM_TICK_TYPE_SPAWN;
    ent->think.coreTickType = SIM_TICK_TYPE_SEEKER;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = Sim_CalcThinkTick(scene, 1.5f);
    ent->deathType = SIM_DEATH_GFX_GIB;
    ent->basePriority = 8;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitSeekerFlying(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntLife(ent, NO, YES, 60);
    Sim_SetEntMoveType(ent, &ent->movement, 6, SIM_ENT_MOVE_TYPE_WALK, NO);
    
    Sim_SetEntityBody(ent, { 1.5f, 1.5f, 1.5f });
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 0, 0.7f, 0.7f, 1 },
        { 0, 0.7f, 0.7f, 1 },
        "Cube",
        "Enemy",
        SIM_DEATH_GFX_GIB);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    ent->think.tickType = SIM_TICK_TYPE_SPAWN;
    ent->think.coreTickType = SIM_TICK_TYPE_SEEKER_FLYING;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = Sim_CalcThinkTick(scene, 1.5f);
    ent->deathType = SIM_DEATH_GFX_GIB;
    ent->basePriority = 8;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitGrunt(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* data)
{
    Sim_SetEntityBase(ent, data);
    Sim_SetEntLife(ent, NO, YES, 100);
    Sim_SetEntMoveType(ent, &ent->movement, 3, SIM_ENT_MOVE_TYPE_WALK, NO);
    
    Sim_SetEntityBody(ent, { 1.5, 1, 1.5 });
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 0.2f, 0.2f, 1 },
        { 1, 0.2f, 0.2f, 1 },
        "Cube",
        "Enemy",
        SIM_DEATH_GFX_GIB);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    ent->think.tickType = SIM_TICK_TYPE_SPAWN;
    ent->think.coreTickType = SIM_TICK_TYPE_GRUNT;
    ent->timing.lastThink = ent->timing.birthTick;
    ent->timing.nextThink = Sim_CalcThinkTick(sim, 1.5f);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitBrute(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* data)
{
    ZE_ASSERT(0, "Not implemented");
    Sim_SetEntityBase(ent, data);
    Sim_SetEntLife(ent, NO, YES, 100);
    Sim_SetEntMoveType(ent, &ent->movement, 3, SIM_ENT_MOVE_TYPE_WALK, NO);
    
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    Sim_SetEntityBody(ent, { 3.0f, 2.0f, 3.0f });
    Sim_SetEnemyDefaultFlags(ent);
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 0.2f, 0.2f, 1 },
        { 1, 0.2f, 0.2f, 1 },
        "Cube",
        "Enemy",
        SIM_DEATH_GFX_GIB);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitCharger(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* data)
{
    ZE_ASSERT(0, "Not implemented");
    Sim_SetEntityBase(ent, data);
    Sim_SetEntLife(ent, NO, YES, 100);
    Sim_SetEntMoveType(ent, &ent->movement, 3, SIM_ENT_MOVE_TYPE_WALK, NO);
    ent->teamId = SIM_ENT_TEAM_ENEMY;
    Sim_SetEntityBody(ent, { 2.0f, 2.0f, 2.0f });
    Sim_SetEnemyDefaultFlags(ent);
    return ZE_ERROR_NONE;
}

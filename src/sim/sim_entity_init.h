#pragma once

#include "sim.h"


////////////////////////////////////////////////////////////////////
// Shared Entity initialisation
////////////////////////////////////////////////////////////////////
internal void Sim_SetEntityBase(
    SimEntity* ent, SimEvent_Spawn* def)
{
    ent->timing.birthTick = def->birthTick;
    ent->body.t.pos = def->pos;
    ent->body.previousPos = def->pos;
    ent->movement.move = def->move;
    ent->movement.velocity = def->velocity;
    ent->relationships.childFactoryType = def->childFactoryType;
    ent->relationships.parentId.serial = def->parentSerial;
    ent->priority = 1;
    ent->basePriority = 1;
    ent->life.stunDuration = 1;
    Transform_SetScaleSafe(&ent->body.t, def->scale);
}

internal void Sim_SetEntLife(SimEntity* ent, i32 bInvulnerable, i32 bIsShootable, i32 startHealth)
{
    // eg: a wall is shootable, but it is also invulernable.
    IF_TO_BIT(bIsShootable, ent->flags, SIM_ENT_FLAG_SHOOTABLE)
    IF_TO_BIT(bInvulnerable, ent->flags, SIM_ENT_FLAG_INVULNERABLE)
    ent->life.health = startHealth;
    ent->life.healthMax = startHealth;
}

internal void Sim_SetEntMoveType(
    SimEntity* ent, SimEntMovement* mover, f32 moveSpeed, i32 moveType, i32 bAvoidOthers)
{
    IF_TO_BIT(bAvoidOthers, ent->flags, SIM_ENT_FLAG_MOVE_AVOID)
    mover->moveMode = moveType;
    mover->speed = moveSpeed;
}

// internal void Sim_SetEntityStats(
//     SimEntity* ent,
//     f32 speed,
//     i32 health,
//     f32 attackTime)
// {
//     ent->movement.speed = speed;
//     ent->life.health = health;
//     ent->attackTime = attackTime;
// }

internal void Sim_SetEntityDisplay_Mesh(
    SimEntity* ent,
    Colour colourA,
    Colour colourB,
    char* meshName,
    char* materialName,
    u8 deathType)
{
    ZRAssetDB* db = App_GetAssetDB();
    
    //ent->display.prefabIndex = prefabIndex;
    ent->display.data.type = ZR_DRAWOBJ_TYPE_MESH;
    ent->display.data.model.meshIndex = db->GetMeshByName(db, meshName)->header.index;
	ent->display.data.model.materialIndex = db->GetMaterialByName(db, materialName)->header.index;
    
    ent->deathType = deathType;
    ent->display.colourA = colourA;
    ent->display.colourB = colourB;
}

internal void Sim_SetEntityBody(
    SimEntity* ent, Vec3 size)
{
    ent->body.t.scale = size;
    ent->body.baseHalfSize.x = size.x / 2;
    ent->body.baseHalfSize.y = size.y / 2;
    ent->body.baseHalfSize.z = size.z / 2;
}

internal void Sim_SetEnemyDefaultFlags(SimEntity* ent)
{
    ent->flags =
          SIM_ENT_FLAG_SHOOTABLE
        | SIM_ENT_FLAG_POSITION_SYNC
        | SIM_ENT_FLAG_MOVE_AVOID
        | SIM_ENT_FLAG_OUT_OF_PLAY;
}

#pragma once

#include "sim.h"


////////////////////////////////////////////////////////////////////
// Shared Entity initialisation
////////////////////////////////////////////////////////////////////
internal void Sim_SetEntityBase(
    SimEntity* ent, SimEntSpawnData* def)
{
    ent->timing.birthTick = def->birthTick;
    ent->body.t.pos = def->pos;
    ent->body.previousPos = def->pos;
    ent->destination = def->destination;
    ent->body.velocity = def->velocity;
    ent->relationships.childFactoryType = def->childFactoryType;
    ent->relationships.parentId.serial = def->parentSerial;
    ent->priority = 1;
    ent->basePriority = 1;
    Transform_SetScaleSafe(&ent->body.t, def->scale);
}

internal void Sim_SetEntityStats(
    SimEntity* ent, f32 speed, i16 health, f32 attackTime)
{
    ent->body.speed = speed;
    ent->life.health = health;
    ent->attackTime = attackTime;
}

internal void Sim_SetEntityDisplay(
    SimEntity* ent,
    Colour colourA,
    Colour colourB,
    i32 meshIndex,
    u8 deathType)
{
    ent->display.meshIndex = meshIndex;
    ent->deathType = deathType;
    ent->display.colourA = colourA;
    ent->display.colourB = colourB;
}


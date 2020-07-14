#ifndef SIM_ENTITY_TICKS_H
#define SIM_ENTITY_TICKS_H

#include "sim.h"
/////////////////////////////////////////////////////////
// Utility functions for entity ticks
/////////////////////////////////////////////////////////
/**
 * Returns a target if the entity has one, NULL if not
 */
internal SimEntity* SimEnt_UpdateTargetting(SimScene* sim, SimEntity* ent, i32 bIsServer)
{
    SimEntity* target = NULL;
	if (bIsServer == YES)
    {
        // acquire/check target
        target = Sim_FindTargetForEnt(sim, ent);
    }
    else if (ent->relationships.targetId.serial != 0)
    {
        // check by target Id
        target = Sim_GetEntityBySerial(sim, ent->relationships.targetId.serial);
    }
    return target;
}

/////////////////////////////////////////////////////////
// Entity tick functions shared between client and server
/////////////////////////////////////////////////////////
internal i32 SimEnt_TickTimeout(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        //Sim_RemoveEntity(sim, ent->id.serial);
        Sim_WriteRemoveEntity(sim, ent, NULL, SIM_DEATH_STYLE_TIMEOUT, {}, 0);
    }
    return ZE_ERROR_NONE;
}

internal i32 SimEnt_TickSpawnAnimation(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    Vec3* halfSize = &ent->body.baseHalfSize;
    if (sim->tick >= ent->timing.nextThink)
    {
        ent->flags &= ~SIM_ENT_FLAG_OUT_OF_PLAY;
        ent->tickType = ent->coreTickType;
        ent->body.t.scale = { halfSize->x * 2, halfSize->y * 2, halfSize->z * 2 };
    }
    else
    {
        ent->flags |= SIM_ENT_FLAG_OUT_OF_PLAY;
        frameInt totalWait = ent->timing.nextThink - ent->timing.lastThink;
        frameInt progress = sim->tick - ent->timing.lastThink;
        
        f32 time = (f32)progress / (f32)totalWait;
        ent->body.t.scale.x = ZE_LerpF32(0.001f, halfSize->x * 2, time);
        ent->body.t.scale.y = ZE_LerpF32(50.0f, halfSize->y * 2, time);
        ent->body.t.scale.z = ZE_LerpF32(0.001f, halfSize->z * 2, time);
    }
    return ZE_ERROR_NONE;
}

internal void SimEnt_TickStun(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        ent->tickType = ent->coreTickType;
    }
}

internal void SimEnt_TickSeeker(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    SimEntity* target = SimEnt_UpdateTargetting(sim, ent, bIsServer);
    // Have we got a target?
    if (target == NULL)
    {
        ent->movement.velocity = { 0, 0, 0 };
    }
    else
    {
        Vec3 toTarget =
        {
            target->body.t.pos.x - ent->body.t.pos.x,
            0,
            target->body.t.pos.z - ent->body.t.pos.z
        };
        Vec3_Normalise(&toTarget);
        if (ent->flags & SIM_ENT_FLAG_MOVE_AVOID)
        {
            SimAvoidInfo avoid = Sim_BuildAvoidVector(sim, ent, 1);
            // Multiply by number of neighbours found
            // to scale move vector up when surrounded
            toTarget.x += avoid.dir.x * avoid.numNeighbours;
            toTarget.y += 0,
            toTarget.z += avoid.dir.z * avoid.numNeighbours;
            Vec3_Normalise(&toTarget);
        }
        ent->movement.velocity =
        {
            toTarget.x * ent->movement.speed,
            0,
            toTarget.z * ent->movement.speed,
        };
    }
    Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

internal void SimEnt_TickSeekerFlying(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    SimEntity* target = SimEnt_UpdateTargetting(sim, ent, bIsServer);
    // Have we got a target?
    if (target == NULL)
    {
        ent->movement.velocity = { 0, 0, 0 };
    }
    else
    {
        Vec3 toTarget =
        {
            target->body.t.pos.x - ent->body.t.pos.x,
            target->body.t.pos.y - ent->body.t.pos.y,
            target->body.t.pos.z - ent->body.t.pos.z
        };
        Vec3_Normalise(&toTarget);
        if (ent->flags & SIM_ENT_FLAG_MOVE_AVOID)
        {
            SimAvoidInfo avoid = Sim_BuildAvoidVector(sim, ent, 1);
            // Multiply by number of neighbours found
            // to scale move vector up when surrounded
            toTarget.x += avoid.dir.x * avoid.numNeighbours;
            toTarget.y += avoid.dir.y * avoid.numNeighbours;
            toTarget.z += avoid.dir.z * avoid.numNeighbours;
            Vec3_Normalise(&toTarget);
        }
        ent->movement.velocity =
        {
            toTarget.x * ent->movement.speed,
            toTarget.y * ent->movement.speed,
            toTarget.z * ent->movement.speed,
        };
    }
    Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

internal void SimEnt_TickDart(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
	Vec3 previousPos = ent->body.t.pos;
    Sim_SimpleMove(ent, deltaTime);
    if (!Sim_InBounds(ent, &sim->boundaryMin, &sim->boundaryMax))
    {
        ent->movement.velocity.x *= -1;
        ent->movement.velocity.y *= -1;
        ent->movement.velocity.z *= -1;
        ent->body.t.pos = previousPos;
    }
}

internal void SimEnt_TickBouncer(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
	Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

internal void SimEnt_TickWanderer(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
	if (ent->timing.nextThink >= sim->tick)
    {
        ent->timing.lastThink = ent->timing.nextThink;
        
        ent->timing.nextThink += (i32)COM_STDRandomInRange(
            (f32)App_CalcTickInterval(1),
            (f32)App_CalcTickInterval(6)
        );
        f32 radians = COM_STDRandomInRange(0, 360) * DEG2RAD;
        ent->movement.velocity.x = cosf(radians) * ent->movement.speed;
        ent->movement.velocity.z = sinf(radians) * ent->movement.speed;
    }
    Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

internal void SimEnt_TickSpawner(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    //printf("Update spawner\n");
    i32 spawnSpaces = ent->relationships.maxLiveChildren - 
        ent->relationships.liveChildren;
    if (spawnSpaces < ent->relationships.childSpawnCount)
    { return; }

    if (sim->tick >= ent->timing.nextThink)
    {
        ent->timing.lastThink = ent->timing.nextThink;
        ent->timing.nextThink += App_CalcTickInterval(2);
        ent->relationships.liveChildren += 
            ent->relationships.childSpawnCount;
        
        // Write spawn cmd
        SimEvent_BulkSpawn event = {};
        Transform t;
        t.pos = ent->body.t.pos;
        M3x3_SetToIdentity(t.rotation.cells);
        t.scale = { 1, 1, 1 };
        Sim_SetBulkSpawn(
            &event,
            Sim_ReserveEntitySerials(sim, 0, ent->relationships.childSpawnCount),
            ent->id.serial,
            t,
            sim->tick,
            ent->relationships.childFactoryType,
            ent->relationships.patternType,
            (u8)ent->relationships.childSpawnCount,
            COM_STDRandU8(),
            10.0f,
            0
        );
        ZCmd_Write(&event.header, &sim->tempOutput->cursor);
    }
}

internal void Sim_TickEntities(SimScene* sim, ZEByteBuffer* output, timeFloat delta)
{
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
		
        const i32 bIsServer = (sim->flags & SIM_SCENE_BIT_IS_SERVER) > 0;
	    switch (ent->tickType)
        {
	    	case SIM_TICK_TYPE_PROJECTILE:
            { SimEnt_TickProjectile(sim, ent, delta, bIsServer); } break;
            case SIM_TICK_TYPE_EXPLOSION:
            { SimEnt_TickTimeout(sim, ent, delta); } break;
	    	case SIM_TICK_TYPE_ACTOR:
            { SimEnt_TickActor(sim, ent, delta, bIsServer); } break;
            // case SIM_TICK_TYPE_BOT:
            // { SVG_UpdateBot(sim, ent, delta); } break;
            case SIM_TICK_TYPE_SPAWNER:
            { SimEnt_TickSpawner(sim, ent, delta, bIsServer); } break;
            case SIM_TICK_TYPE_SEEKER:
            { SimEnt_TickSeeker(sim, ent, delta, bIsServer); } break;
            case SIM_TICK_TYPE_SEEKER_FLYING:
	    	{ SimEnt_TickSeekerFlying(sim, ent, delta, bIsServer); } break;
	    	case SIM_TICK_TYPE_WANDERER:
            { SimEnt_TickWanderer(sim, ent, delta, bIsServer); break; }
            case SIM_TICK_TYPE_BOUNCER:
            { SimEnt_TickBouncer(sim, ent, delta, bIsServer); } break;
            case SIM_TICK_TYPE_DART:
            { SimEnt_TickDart(sim, ent, delta, bIsServer); } break;
            case SIM_TICK_TYPE_LINE_TRACE:
            { SimEnt_TickTimeout(sim, ent, delta); } break;
            case SIM_TICK_TYPE_SPAWN:
            { SimEnt_TickSpawnAnimation(sim, ent, delta); } break;
            case SIM_TICK_TYPE_WORLD: { } break;
            case SIM_TICK_TYPE_NONE: { } break;
            default:
            //{ ZE_ASSERT(0, "Unknown Ent Tick Type"); } break;
            { printf("Unknown Ent Tick Type %d\n", ent->tickType); } break;
        }

        // make sure previous positions are updated
        ent->body.previousPos = ent->body.t.pos;
    }
    sim->tick++;
    sim->time += delta;
}

#endif // SIM_ENTITY_TICKS_H
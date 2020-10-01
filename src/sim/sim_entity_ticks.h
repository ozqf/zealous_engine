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
internal void SimEnt_TickTimeout(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        //Sim_RemoveEntity(sim, ent->id.serial);
        Sim_WriteRemoveEntity(sim, ent, NULL, SIM_DEATH_STYLE_TIMEOUT, {}, 0);
    }
}

internal void SimEnt_TickSpawnAnimation(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    Vec3* halfSize = &ent->body.baseHalfSize;
    if (sim->tick >= ent->timing.nextThink)
    {
        ent->flags &= ~SIM_ENT_FLAG_OUT_OF_PLAY;
        ent->think.tickType = ent->think.coreTickType;
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
}

internal void SimEnt_TickStun(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
	i32 bIgnoreTimeout = NO;
	// ground based enemies remain stunned until they fall to the ground
	if (ent->movement.moveMode == SIM_ENT_MOVE_TYPE_WALK
		&& !IF_BIT(ent->movement.flags, SIM_ENT_MOVE_BIT_GROUNDED))
	{ bIgnoreTimeout = YES; }

    if (!bIgnoreTimeout && sim->tick >= ent->timing.nextThink)
    {
		// leave next tick alone so new tick type "tocks" immediataly.
        ent->think.tickType = ent->think.coreTickType;
    }
	if (ent->movement.moveMode == SIM_ENT_MOVE_TYPE_WALK)
	{
		Sim_MoveThrown(sim, ent, deltaTime);
	}
}

internal void SimEnt_TickBouncer(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    
    // check for movement
    if (IF_BIT(ent->movement.flags, SIM_ENT_MOVE_BIT_GROUNDED))
    {
        SimEntMovement* mov = &ent->movement;
        mov->velocity.x = mov->move.x * mov->speed;
        mov->velocity.z = mov->move.z * mov->speed;
    }
	Sim_SimpleMove(sim, ent, &ent->movement, deltaTime);
    // update desired move to velocity in case we bounced off something.
    ent->movement.move = Vec3_Normalised(ent->movement.velocity);
}

//internal void SimEnt_TickWanderer(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
SIM_DEFINE_ENT_UPDATE_FN(SimEnt_TickWanderer)
{
    SimEntMovement* mover = &ent->movement;
	if (ent->timing.nextThink <= sim->tick)
    {
        ent->timing.lastThink = ent->timing.nextThink;
        
        f32 randomWait = COM_STDRandomInRange(1, 6);
        ent->timing.nextThink = Sim_CalcThinkTick(sim, randomWait);
        f32 radians = COM_STDRandomInRange(0, 360) * DEG2RAD;
        ent->movement.move = 
        {
            cosf(radians),
            0,
            sinf(radians)
        };
        mover->velocity.x = mover->move.x * mover->speed;
        mover->velocity.z = mover->move.z * mover->speed;
    }
    Sim_SimpleMove(sim, ent, &ent->movement, deltaTime);
}

SIM_DEFINE_ENT_UPDATE_FN(SimEnt_TickGrunt)
{
    SimEntity* target = SimEnt_UpdateTargetting(sim, ent, bIsServer);
    // Have we got a target?
    if (target == NULL)
    {
        ent->movement.velocity = { 0, 0, 0 };
    }
    else
    {
        if (ent->timing.nextThink <= sim->tick
            && (ent->movement.flags & SIM_ENT_MOVE_BIT_GROUNDED) != 0)
        {
            ent->timing.lastThink = ent->timing.nextThink;

            f32 randomWait = COM_STDRandomInRange(1, 1);
            ent->timing.nextThink = Sim_CalcThinkTick(sim, randomWait);
            ent->think.tickType = SIM_TICK_TYPE_PROJECTILE_ATTACK;
            ent->think.subMode = SIM_TICK_SUBMODE_NONE;
        }
        else
        {
            ent->movement.velocity = SimEnt_CalcFlatVelocityTowardPos(
                sim, ent, target->body.t.pos);
        }
    }
    
    Vec3 rot = Vec3_EulerAngles(Vec3_Flipped(ent->movement.velocity));
    Transform_SetRotation(&ent->body.t, 0, rot.y, 0);

    Sim_SimpleMove(sim, ent, &ent->movement, deltaTime);
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
        ent->movement.velocity = SimEnt_CalcFlatVelocityTowardPos(
            sim, ent, target->body.t.pos);
        
        #if 0
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
        #endif
    }
    Sim_SimpleMove(sim, ent, &ent->movement, deltaTime);

    Vec3 rot = Vec3_EulerAngles(ent->movement.velocity);
    Transform_SetRotation(&ent->body.t, 0, rot.y, 0);
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
    Sim_SimpleMove(sim, ent, &ent->movement, deltaTime);
}

internal void SimEnt_TickDart(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
	Vec3 previousPos = ent->body.t.pos;
    Sim_SimpleMove(sim, ent, &ent->movement, deltaTime);
    if (!Sim_InBounds(ent, &sim->boundaryMin, &sim->boundaryMax))
    {
        ent->movement.velocity.x *= -1;
        ent->movement.velocity.y *= -1;
        ent->movement.velocity.z *= -1;
        ent->body.t.pos = previousPos;
    }
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
        ent->timing.nextThink = Sim_CalcThinkTick(sim, 2);
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
            SIM_ENT_TEAM_ENEMY,
            ent->relationships.patternType,
            (u8)ent->relationships.childSpawnCount,
            COM_STDRandU8(),
            10.0f,
            0
        );
        ZCmd_Write(&event.header, &sim->outputBuf->cursor);
    }
}

internal void Sim_InitTickFunctions(SimScene* sim)
{
    for (i32 i = 0; i < SIM_MAX_ENT_UPDATERS; ++i)
    {
        g_entUpdaters[i] = NULL;
    }
    g_entUpdaters[SIM_TICK_TYPE_NONE] = NULL;
    g_entUpdaters[SIM_TICK_TYPE_GRUNT] = SimEnt_TickBouncer;
    g_entUpdaters[SIM_TICK_TYPE_PROJECTILE] = SimEnt_TickProjectile;

    g_entUpdaters[SIM_TICK_TYPE_EXPLOSION] = SimEnt_TickTimeout;
	g_entUpdaters[SIM_TICK_TYPE_ACTOR] = SimEnt_TickActor;
    //g_entUpdaters[SIM_TICK_TYPE_BOT] = SVG_UpdateBot(sim, ent, delta);

    g_entUpdaters[SIM_TICK_TYPE_STUN] = SimEnt_TickStun;
    g_entUpdaters[SIM_TICK_TYPE_SPAWNER] = SimEnt_TickSpawner;
    g_entUpdaters[SIM_TICK_TYPE_SEEKER] = SimEnt_TickSeeker;
    g_entUpdaters[SIM_TICK_TYPE_SEEKER_FLYING] = SimEnt_TickSeekerFlying;
	g_entUpdaters[SIM_TICK_TYPE_WANDERER] = SimEnt_TickWanderer;
    g_entUpdaters[SIM_TICK_TYPE_BOUNCER] = SimEnt_TickBouncer;
    g_entUpdaters[SIM_TICK_TYPE_DART] = SimEnt_TickDart;
    g_entUpdaters[SIM_TICK_TYPE_LINE_TRACE] = SimEnt_TickTimeout;
    g_entUpdaters[SIM_TICK_TYPE_SPAWN] = SimEnt_TickSpawnAnimation;
    g_entUpdaters[SIM_TICK_TYPE_GRUNT] = SimEnt_TickGrunt;
    g_entUpdaters[SIM_TICK_TYPE_PROJECTILE_ATTACK] = SimEnt_TickProjectileAttack;
    //g_entUpdaters[SIM_TICK_TYPE_GRUNT] = SimEnt_TickBouncer;
}

internal SimEntUpdate Sim_GetTickFunc(i32 index)
{
    if (index < 0 || index >= SIM_MAX_ENT_UPDATERS)
    {
        printf("No ent updater for index %d\n", index);
        return NULL;
    }
    return g_entUpdaters[index];
}

internal void Sim_TickEntities(SimScene* sim, ZEBuffer* output, timeFloat delta)
{
    i32 bIsServer = (sim->flags & SIM_SCENE_BIT_IS_SERVER) > 0;
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
        
        // make sure previous positions are updated
        ent->body.previousPos = ent->body.t.pos;
		
        #if 1
        SimEntUpdate updater = Sim_GetTickFunc(ent->think.tickType);
        if (updater == NULL) { continue; }
        updater(sim, ent, delta, bIsServer);
        #endif
    }
    sim->tick++;
    sim->time += delta;
}

#endif // SIM_ENTITY_TICKS_H
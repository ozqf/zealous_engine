#pragma once

#include "client_internal.h"

#include <math.h>

// For debugging - the only time enemies change is on server update
#define CLG_NO_ENEMY_TICK_DROPOUT { if ((g_clDebugFlags & CL_DEBUG_FLAG_NO_ENEMY_TICK)) return; }

internal void CLG_SpawnLineSegment(SimScene* sim, Vec3 origin, Vec3 dest)
{
    SimEntSpawnData def = {};
    def.factoryType = SIM_FACTORY_TYPE_LINE_TRACE;
    def.serial = Sim_ReserveEntitySerial(sim, 1);
    def.isLocal = 1;
    def.pos = origin;
    def.destination = dest;
    Sim_RestoreEntity(sim, &def);
}

internal void CLG_HandleEntityDeath(SimScene* sim, i32 serial)
{
    SimEntity* ent = Sim_GetEntityBySerial(sim, serial);
    if (!ent) { return; }
    switch (ent->deathType)
    {
        case SIM_DEATH_GFX_EXPLOSION:
        {
            SimEntSpawnData def = {};
            def.factoryType = SIM_FACTORY_TYPE_BULLET_IMPACT;
            def.birthTick = sim->tick;
            def.pos = ent->body.t.pos;
            def.serial = Sim_ReserveEntitySerial(sim, 1);
            Sim_RestoreEntity(sim, &def);
        } break;
    }
}

#define CLG_DEFINE_ENT_UPDATE(entityTypeName) internal void \
    CLG_Update##entityTypeName##(SimScene* sim, SimEntity* ent, timeFloat deltaTime)

CLG_DEFINE_ENT_UPDATE(Explosion)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        Sim_RemoveEntity(sim, ent->id.serial);
    }
}

CLG_DEFINE_ENT_UPDATE(Projectile)
{
	frameInt numSteps = 1 + ent->timing.fastForwardTicks;
    ent->timing.fastForwardTicks = 0;
    //APP_LOG(64, "CL Prj %d steps: %d\n", ent->id.serial, numSteps);
	if (numSteps < 1) { numSteps = 1; }
	while (numSteps)
	{
		numSteps--;
		Sim_SimpleMove(ent, deltaTime);
        
		/*if (sim->tick >= ent->timing.nextThink)
		{
			Sim_RemoveEntity(sim, ent->id.serial);
			return;
		}*/
	}
    f32 yaw = atan2f(-ent->movement.velocity.z, ent->movement.velocity.x);
    yaw -= 90.0f * DEG2RAD;
    f32 pitch = 0;//90.0f * DEG2RAD;
    M3x3_SetToIdentity(ent->body.t.rotation.cells);
    Transform_SetRotation(&ent->body.t, pitch, yaw, 0);
}

internal void CLG_StepActor(
    SimScene* sim,
    SimEntity* ent,
    SimActorInput* input,
    timeFloat interval)
{
    SimEnt_StepActorMovement(sim, ent, input, interval);
    #if 0
    f32 deltaTime = (f32)interval;
    Vec3 move = {};
	f32 speed = ent->movement.speed;//5.0f;
	if (input->buttons & ACTOR_INPUT_MOVE_FORWARD)
	{
		move.z -= speed * deltaTime;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_BACKWARD)
	{
		move.z += speed * deltaTime;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_LEFT)
	{
		move.x -= speed * deltaTime;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_RIGHT)
	{
		move.x += speed * deltaTime;
	}
	ent->body.previousPos = ent->body.t.pos;
	ent->body.t.pos.x += move.x;
	ent->body.t.pos.y += move.y;
	ent->body.t.pos.z += move.z;
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
    #endif
}

internal void CLG_FireActorAttack(SimScene* sim, SimEntity* ent, Vec3* dir)
{
    /* Debug - Fire a local projectile to see how it matches the server */
    #if 0
    SimBulkSpawnEvent def = {};
    def.factoryType = SIM_FACTORY_TYPE_PROJ_PREDICTION;
    def.base.firstSerial = Sim_ReserveEntitySerial(sim, 1);
    def.base.pos = ent->body.t.pos;
    def.base.seedIndex = 0;
    def.base.forward = *dir;
    def.base.tick = g_ticks;
    //Sim_ExecuteEnemySpawn()
    Sim_ExecuteBulkSpawn(sim, &def, 0);
    #endif
	
    /* Debug - Create a line trace */
    #if 0
    Vec3 origin = ent->body.t.pos;
    Vec3 dest {};
    dest.x = (origin.x + (dir->x * 10));
    dest.y = (origin.y + (dir->y * 10));
    dest.z = (origin.z + (dir->z * 10));
    CLG_SpawnLineSegment(sim, origin, dest);
    #endif
}

CLG_DEFINE_ENT_UPDATE(Actor)
{
    // -1 as sequence has been incremented before sim update
    //APP_PRINT(128, "CL actor update and step Seq %d\n", (g_userInputSequence - 1));
    // Movement
    CLG_StepActor(sim, ent, &ent->input, deltaTime); 
    
    //////////////////////////////////////////////////////////////
    // Predicted shooting
    //////////////////////////////////////////////////////////////
    
    if (ent->attackTick <= 0)
    {
        Vec3 shoot {};
        if (ent->input.buttons & ACTOR_INPUT_SHOOT_LEFT)
        {
            shoot.x -= 1;
        }
        if (ent->input.buttons & ACTOR_INPUT_SHOOT_RIGHT)
        {
            shoot.x += 1;
        }
        if (ent->input.buttons & ACTOR_INPUT_SHOOT_UP)
        {
            shoot.z -= 1;
        }
        if (ent->input.buttons & ACTOR_INPUT_SHOOT_DOWN)
        {
            shoot.z += 1;
        }
        if (shoot.x != 0 || shoot.z != 0)
        {
            ent->attackTick = ent->attackTime;
            Vec3_Normalise(&shoot);
			CLG_FireActorAttack(sim, ent, &shoot);
        }
    }
    else
    {
        ent->attackTick -= deltaTime;
    }
}

CLG_DEFINE_ENT_UPDATE(LineTrace)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        Sim_RemoveEntity(sim, ent->id.serial);
    }
}

internal void CLG_UpdateTargetPoint(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    SimEntity* plyr = Sim_GetEntityBySerial(sim, g_avatarSerial);
    if (plyr == NULL) { return; }
    Vec3 entPos = plyr->body.t.pos;
    Vec3 forward = plyr->body.t.rotation.zAxis;
    Vec3 dest;
    dest.x = entPos.x + (-forward.x * 50);
    dest.y = entPos.y + (-forward.y * 50);
    dest.z = entPos.z + (-forward.z * 50);

    const i32 max_overlaps = 32;
    SimRaycastResult results[max_overlaps];
    i32 overlaps = 0;
    overlaps = Sim_FindByRaycast(
        sim, entPos, dest, {}, ent->id.serial, results, max_overlaps);
    i32 hitIndex = Sim_FindClosestRayhit(results, overlaps);
    //App_DebugBreak();
    if (hitIndex >= 0)
    {
        dest = results[hitIndex].hitPos;
        if (results[hitIndex].ent == NULL)
        {
            printf("Hit index %d entity is null!\n", hitIndex);
        }
    }
    #if 0
    for (i32 i = 0; i < overlaps; ++i)
    {
        SimEntity* victim = results[i].ent;
        if (Sim_IsEntTargetable(victim) == NO) { continue; }

        dest = results[i].hitPos;
        break;
    }
    #endif
    ent->body.t.pos = dest;
}

internal void CLG_TickEntity(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
	const i32 bIsServer = NO;
    switch (ent->tickType)
    {
        case SIM_TICK_TYPE_PROJECTILE:
        { CLG_UpdateProjectile(sim, ent, deltaTime); } break;
		case SIM_TICK_TYPE_BOUNCER:
        { SimEnt_TickBouncer(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_SEEKER:
        { SimEnt_TickSeeker(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_SEEKER_FLYING:
		{ SimEnt_TickSeekerFlying(sim, ent, deltaTime, bIsServer); } break;
		case SIM_TICK_TYPE_WANDERER:
        { SimEnt_TickWanderer(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_DART:
        { SimEnt_TickDart(sim, ent, deltaTime, bIsServer); } break;
		case SIM_TICK_TYPE_ACTOR:
        { CLG_UpdateActor(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SPAWN:
        { SimEnt_TickSpawnAnimation(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_TARGET_POINT:
        { CLG_UpdateTargetPoint(sim, ent, deltaTime); break; }
        case SIM_TICK_TYPE_LINE_TRACE:
        { CLG_UpdateLineTrace(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_EXPLOSION:
        { CLG_UpdateExplosion(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_GRUNT: { } break;
        case SIM_TICK_TYPE_BOT: { } break;
        case SIM_TICK_TYPE_WORLD: { } break;
        case SIM_TICK_TYPE_NONE: { } break;
        default: { ZE_ASSERT(0, "Client - unknown entity tick type") } break;
    }
}

internal void CLG_TickViewSway(timeFloat deltaTime)
{
    SimEntity* plyr = Sim_GetEntityBySerial(&g_sim, g_avatarSerial);
    if (plyr == NULL)
    {
        g_swayYOffset = 0;
        return;
    }
    f32 speed = Vec3_Magnitude(&plyr->movement.velocity);
    f32 speedSwayMul = speed / ACTOR_BASE_SPEED; // ratio of max speed
    f32 swaySpeed = 4.f * speedSwayMul; // scale max sway speed by ratio
    printf("Speed mul %.3f, Speed %.3f\n", speedSwayMul, swaySpeed);
    
    g_swayTick += (f32)deltaTime * swaySpeed;
    g_swayYOffset = sinf(g_swayTick) * 0.5f;
}

internal void CLG_TickGame(SimScene* sim, timeFloat deltaTime)
{
    AppTimer timer(APP_STAT_CL_SIM, sim->tick);
    for (i32 i = 0; i < g_sim.maxEnts; ++i)
    {
        SimEntity* ent = &g_sim.ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }

        CLG_TickEntity(sim, ent, deltaTime);
    }
    CLG_TickViewSway(deltaTime);
    sim->tick++;
}

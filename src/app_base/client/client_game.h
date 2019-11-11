#pragma once

#include "client.h"

#include <math.h>

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
            def.factoryType = SIM_FACTORY_TYPE_EXPLOSION;
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
        
		if (sim->tick >= ent->timing.nextThink)
		{
			Sim_RemoveEntity(sim, ent->id.serial);
			return;
		}
	}
    f32 yaw = atan2f(-ent->body.velocity.z, ent->body.velocity.x);
    yaw -= 90.0f * DEG2RAD;
    f32 pitch = 0;//90.0f * DEG2RAD;
    M3x3_SetToIdentity(ent->body.t.rotation.cells);
    Transform_SetRotation(&ent->body.t, pitch, yaw, 0);
}

// Return 1 if the command was successfully executed.
internal i32 CLG_SyncEntity(SimScene* sim, S2C_EntitySync* cmd)
{
    i32 executed = 0;
    SimEntity* ent = Sim_GetEntityBySerial(&g_sim, cmd->networkId);
    if (!ent)
    {
        // Must return executed or this dead command will stay
        // in the buffer!
        // ... actually
        // This isn't a bug, and could happen naturally due to
        // out-of-order updates vs in order reliable updates
        // If it is a 'death' sync, we need to hang onto it
        // so that it can be refired when the entity has been spawned.
        // Because entity spawns are always in order and reserved,
        // use the highest spawned Id and compare
        if (cmd->subType == S2C_ENTITY_SYNC_TYPE_DEATH)
        {
            if (cmd->networkId > g_sim.highestAssignedSequence)
            {
                //printf("CL GHOST WARNING\n");
                //return 1;
                //printf("CL Death of %d postponed (highest %d)!\n",
                //    cmd->networkId, g_sim.highestAssignedSequence);
                return 0;
            }
            //printf("CL Death repeat of %d, dicarding\n", cmd->networkId);
        }
        return 1;
        #if 0
        //APP_PRINT(128, "CL No ent %d for sync\n", cmd->networkId);
        ZE_ASSERT(cmd->type != S2C_ENTITY_SYNC_TYPE_DEATH,
            "CL death sync but no entity!\n");
        
        executed = 1;
        #endif
    }
    else
    {
        if (cmd->networkId == g_avatarSerial)
        {
            // TODO: Don't bother sending client avatar syncs
            // Do NOT sync the client's avatar here.
            // There's a special command for that!
            return 1;
        }
        if (cmd->subType == S2C_ENTITY_SYNC_TYPE_UPDATE)
        {
            Vec3 currentPos =
            {
                ent->body.t.pos.x + ent->body.error.x,
                ent->body.t.pos.y + ent->body.error.y,
                ent->body.t.pos.z + ent->body.error.z,
            };
            //ent->body.error.x = cmd->pos.x - currentPos.x;
            //ent->body.error.y = cmd->pos.y - currentPos.y;
            //ent->body.error.z = cmd->pos.z - currentPos.z;
            ent->body.error.x = currentPos.x - cmd->update.pos.x;
            ent->body.error.y = currentPos.y - cmd->update.pos.y;
            ent->body.error.z = currentPos.z - cmd->update.pos.z;
            if (Vec3_Magnitude(&ent->body.error) > 2)
            {
                ent->body.errorRate = 0.8f;    
            }
            else
            {
                ent->body.errorRate = 0.98f;
            }
            
            ent->body.previousPos = ent->body.t.pos;
            ent->body.t.pos = cmd->update.pos;
            ent->priority = cmd->update.priority;
            ent->body.velocity = cmd->update.vel;
            ent->relationships.targetId.serial = cmd->update.targetId;
            ent->clientOnly.lastSync = sim->tick;
        }
        else if (cmd->subType == S2C_ENTITY_SYNC_TYPE_DEATH)
        {
            //if (ent->tickType == SIM_TICK_TYPE_PROJECTILE)
            //{
            //    printf("CL Sync prj death of %d\n", cmd->networkId);
            //}
            CLG_HandleEntityDeath(&g_sim, cmd->networkId);
            Sim_RemoveEntity(sim, cmd->networkId);
        }
        else
        {
            ZE_ASSERT(0, "Unknown entity sync type\n");
        }
        
        
        executed = 1;
    }
    return executed;
}

internal void CLG_StepActor(
    SimScene* sim,
    SimEntity* ent,
    SimActorInput* input,
    timeFloat interval)
{
    f32 deltaTime = (f32)interval;
    Vec3 move = {};
	f32 speed = ent->body.speed;//5.0f;
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
    APP_PRINT(128, "CL actor update and step Seq %d\n", (g_userInputSequence - 1));
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

CLG_DEFINE_ENT_UPDATE(Spawn)
{
    /*
    if (ent->fastForwardTicks > 0)
    {
        printf("CL Fast forward ent by %d!\n", ent->fastForwardTicks);
    }
    if (ent->thinkTick <= 0)
    {
        ent->tickType = ent->coreTickType;
    }
    else
    {
        ent->thinkTick -= deltaTime;
    }
    */
}


//////////////////////////////////////////////////////
// ENEMIES
//////////////////////////////////////////////////////
CLG_DEFINE_ENT_UPDATE(Wanderer)
{
    CLG_NO_ENEMY_TICK_DROPOUT
    
    Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

CLG_DEFINE_ENT_UPDATE(Seeker)
{
    CLG_NO_ENEMY_TICK_DROPOUT
    i32 targetId = ent->relationships.targetId.serial;
    if (targetId != 0)
    {
        SimEntity* target = Sim_GetEntityBySerial(sim, targetId);
        if (target != NULL)
        {
            Vec3 toTarget = 
            {
                target->body.t.pos.x - ent->body.t.pos.x,
                target->body.t.pos.y - ent->body.t.pos.y,
                target->body.t.pos.z - ent->body.t.pos.z,
            };
            Vec3_Normalise(&toTarget);
            SimAvoidInfo avoid = Sim_BuildAvoidVector(sim, ent);
            toTarget.x += avoid.dir.x * avoid.numNeighbours;
            toTarget.y += avoid.dir.y * avoid.numNeighbours;
            toTarget.z += avoid.dir.z * avoid.numNeighbours;
            Vec3_Normalise(&toTarget);
            ent->body.velocity =
            {
                toTarget.x * ent->body.speed,
                toTarget.y * ent->body.speed,
                toTarget.z * ent->body.speed,
            };
        }
    }
    Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

CLG_DEFINE_ENT_UPDATE(Bouncer)
{
    CLG_NO_ENEMY_TICK_DROPOUT
    Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

CLG_DEFINE_ENT_UPDATE(Dart)
{
    CLG_NO_ENEMY_TICK_DROPOUT
    Vec3 previousPos = ent->body.t.pos;
    Sim_SimpleMove(ent, deltaTime);
    if (!Sim_InBounds(ent, &sim->boundaryMin, &sim->boundaryMax))
    {
        ent->body.velocity.x *= -1;
        ent->body.velocity.y *= -1;
        ent->body.velocity.z *= -1;
        ent->body.t.pos = previousPos;
    }
}

internal void CLG_TickEntity(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    switch (ent->tickType)
    {
        case SIM_TICK_TYPE_PROJECTILE:
        { CLG_UpdateProjectile(sim, ent, deltaTime); } break;
		case SIM_TICK_TYPE_BOUNCER:
        { CLG_UpdateBouncer(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SEEKER:
        { CLG_UpdateSeeker(sim, ent, deltaTime); } break;
		case SIM_TICK_TYPE_WANDERER:
        { CLG_UpdateWanderer(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_DART:
        { CLG_UpdateDart(sim, ent, deltaTime); } break;
		case SIM_TICK_TYPE_ACTOR:
        { CLG_UpdateActor(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SPAWN:
        //{ CLG_UpdateSpawn(sim, ent, deltaTime); } break;
        { Sim_TickSpawn(sim, ent, deltaTime); } break;
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

internal void CLG_TickGame(SimScene* sim, timeFloat deltaTime)
{
    AppTimer timer(APP_STAT_CL_SIM, sim->tick);
    for (i32 i = 0; i < g_sim.maxEnts; ++i)
    {
        SimEntity* ent = &g_sim.ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }

        CLG_TickEntity(sim, ent, deltaTime);
    }
    sim->tick++;
}

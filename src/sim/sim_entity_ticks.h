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
extern "C" i32 SimEnt_TickTimeout(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        Sim_RemoveEntity(sim, ent->id.serial);
    }
    return ZE_ERROR_NONE;
}

extern "C" i32 SimEnt_TickSpawnAnimation(
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

extern "C" void SimEnt_TickStun(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        ent->tickType = ent->coreTickType;
    }
}

extern "C" void SimEnt_TickSeeker(
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

extern "C" void SimEnt_TickSeekerFlying(
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

extern "C" void SimEnt_TickDart(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
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

extern "C" void SimEnt_TickBouncer(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
	Sim_SimpleMove(ent, deltaTime);
    Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}

extern "C" void SimEnt_TickWanderer(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
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

extern "C" void SimEnt_TickSpawner(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    printf("Update spawner\n");
}

internal void SimEnt_UpdateActorLook(
    SimScene* sim,
    SimEntity* ent,
    SimActorInput* input,
    f32 dt)
{
    ent->body.pitchDegrees = input->degrees.x;
    ent->body.yawDegrees = input->degrees.y;
    
    // Apply Rotate
    Transform* t = &ent->body.t;
    M3x3_SetToIdentity(t->rotation.cells);
    M3x3_RotateY(t->rotation.cells, input->degrees.y * DEG2RAD);
    M3x3_RotateX(t->rotation.cells, input->degrees.x * DEG2RAD);

}

internal void SimEnt_UpdateActorWalk(
    SimScene* sim,
    SimEntity* ent,
    SimActorInput* input,
    f32 dt)
{
    // Rotation
    SimEnt_UpdateActorLook(sim, ent, input, dt);

    if (ent->movement.moveTime > 0)
    { ent->movement.moveTime -= dt; }
    
    //f32 stepSpeed = ent->movement.speed * dt;

    ////////////////////////////////////////////////////////////////////
    // Movement
    ////////////////////////////////////////////////////////////////////

    // Gather direction input
    Vec3 dir = {};
    if (input->buttons & ACTOR_INPUT_MOVE_FORWARD)
	{
        dir.z -= 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_BACKWARD)
	{
        dir.z += 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_LEFT)
	{
        dir.x -= 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_RIGHT)
	{
        dir.x += 1;
	}
    // Calculate velocity components
    f32 radiansForward = input->degrees.y * DEG2RAD;
    f32 radiansLeft = (input->degrees.y * DEG2RAD) + (90 * DEG2RAD);
    Vec3 forward;
    forward.x = sinf(radiansForward) * dir.z;
    forward.y = 0;
    forward.z = cosf(radiansForward) * dir.z;

    Vec3 left;
    left.x = sinf(radiansLeft) * dir.x;
    left.y = 0;
    left.z = cosf(radiansLeft) * dir.x;

    Vec3 up = {};

    // Enter evade?
	Vec3 move = {};
    i32 hasDirectionInput = (dir.x != 0 || dir.y != 0 || dir.z != 0);
	if (input->buttons & ACTOR_INPUT_MOVE_SPECIAL1
        && hasDirectionInput
        && ent->movement.moveTime <= 0)
	{
        // calculate evade direction - only horizontal
        // but allows diagonals
        Vec3 evadeForward, evadeLeft;
        evadeForward.x = sinf(radiansForward) * dir.z;
        evadeForward.z = cosf(radiansForward) * dir.z;
        evadeLeft.x = sinf(radiansLeft) * dir.x;
        evadeLeft.z = cosf(radiansLeft) * dir.x;
        Vec3 evadeDir = {};
        evadeDir.x = evadeForward.x + evadeLeft.x;
        evadeDir.z = evadeForward.z + evadeLeft.z;
        Vec3_Normalise(&evadeDir);
        // Begin evade
        ent->movement.moveMode = 1;
		ent->movement.moveTime = ACTOR_EVADE_SECONDS;
        ent->movement.velocity.x = evadeDir.x * ACTOR_EVADE_SPEED;
        ent->movement.velocity.z = evadeDir.z * ACTOR_EVADE_SPEED;
	}
    else
    {
        f32 pushSpeed = ACTOR_MOVE_PUSH_SPEED;
        // normal walk
        // calculate velocity change
        Vec3 velocityPush = {};
        Vec3* vel = &ent->movement.velocity;
        if (hasDirectionInput)
        {
            // push in input direction
            velocityPush.x = forward.x + left.x + up.x;
            velocityPush.z = forward.z + left.z + up.z;
        }
        else
        {
            // set move dir to reverse of current velocity and apply to stop
            velocityPush.x = -vel->x;
            velocityPush.z = -vel->z;
        }
        Vec3_SetMagnitude(&velocityPush, pushSpeed * dt);
        // add move to velocity
        vel->x += velocityPush.x;
        vel->z += velocityPush.z;
        Vec3_CapMagnitude(vel, 0.01f, ent->movement.speed);

        // Update velocity reading
        // - velocity will currently be zero as janky movement. So set it to move before scaling
        // ent->movement.velocity = move;
        // Vec3_SetMagnitude(&ent->movement.velocity, ent->movement.speed);
        // if (hasDirectionInput)
        // {
        //     printf("Spd: %.3f, Ent vel: %.3f, %.3f, %.3f\n",
        //         ent->movement.speed,
        //         ent->movement.velocity.x, ent->movement.velocity.y, ent->movement.velocity.z);
        // }
    }
    // move for this frame
    move.x = ent->movement.velocity.x * dt;
	move.z = ent->movement.velocity.z * dt;
    
    Transform* t = &ent->body.t;
    // record previous position
    ent->body.previousPos = t->pos;
    // Apply
	Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
    SimEnt_MoveVsSolid(sim, ent, move);
}

internal void SimEnt_UpdateActorEvade(
    SimScene* sim,
    SimEntity* ent, 
    SimActorInput* input,
    f32 dt)
{
    // Look
    SimEnt_UpdateActorLook(sim, ent, input, dt);

    // apply movement
    Vec3 move = {};
    move.x = ent->movement.velocity.x * dt;
	move.z = ent->movement.velocity.z * dt;

    ent->body.previousPos = ent->body.t.pos;
	ent->body.t.pos.x += move.x;
	ent->body.t.pos.y += move.y;
	ent->body.t.pos.z += move.z;
	Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);

    // timer
    if (ent->movement.moveTime <= 0)
    {
        ent->movement.velocity = {};
        ent->movement.moveTime = ACTOR_EVADE_RESET_SECONDS;
        ent->movement.moveMode = 0;
    }
    else
    {
        ent->movement.moveTime -= dt;
    }
}
#if 0
internal void SimEnt_UpdateActorWalk_TopDown(
    SimScene* sim,
    SimEntity* ent,
    SimActorInput* input,
    f32 dt)
{
    Vec3 dir = {};
	f32 speed = ent->movement.speed;
	if (input->buttons & ACTOR_INPUT_MOVE_FORWARD)
	{
        dir.z -= 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_BACKWARD)
	{
        dir.z += 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_LEFT)
	{
        dir.x -= 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_RIGHT)
	{
        dir.x += 1;
	}
	Vec3_Normalise(&dir);
    i32 hasDirectionInput = (dir.x != 0 || dir.y != 0 || dir.z != 0);

    if (ent->movement.moveTime > 0)
    { ent->movement.moveTime -= dt; }

	Vec3 move = {};
	if (input->buttons & ACTOR_INPUT_MOVE_SPECIAL1
        && hasDirectionInput
        && ent->movement.moveTime <= 0)
	{
        // Begin evade
        ent->movement.moveMode = 1;
		ent->movement.moveTime = ACTOR_EVADE_SECONDS;
        ent->movement.velocity.x = dir.x * ACTOR_EVADE_SPEED;
        ent->movement.velocity.z = dir.z * ACTOR_EVADE_SPEED;
		move.x = ent->movement.velocity.x * dt;
	    move.z = ent->movement.velocity.z * dt;
	}
    else
    {
        // normal walk
        move.x = dir.x * (speed * dt);
	    move.z = dir.z * (speed * dt);
    }
    
	ent->body.previousPos = ent->body.t.pos;
	ent->body.t.pos.x += move.x;
	ent->body.t.pos.y += move.y;
	ent->body.t.pos.z += move.z;
	Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
}
#endif
/**
 * Input is optional to override the input attached to ent for client prediction
 */
extern "C" void SimEnt_StepActorMovement(
    SimScene* sim,
    SimEntity* ent,
    SimActorInput* input,
    timeFloat deltaTime)
{
    if (input == NULL)
    {
        input = &ent->input;
    }
    f32 dt = (f32)deltaTime;
	switch (ent->movement.moveMode)
	{
		case 0: // Walking
		{
			SimEnt_UpdateActorWalk(sim, ent, input, dt);
		} break;
		
		case 1: // Dodging
		{
			SimEnt_UpdateActorEvade(sim, ent, input, dt);
		} break;
		
		default:
		{
			
		} break;
	}
}

internal void Sim_TickEntities(SimScene* sim, ZEByteBuffer* output, timeFloat delta)
{
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
		
        const i32 bIsServer = YES;
	    switch (ent->tickType)
        {
	    	// case SIM_TICK_TYPE_PROJECTILE:
            // { SVG_UpdateProjectile(sim, ent, delta); } break;
	    	// case SIM_TICK_TYPE_ACTOR:
            // { SVG_UpdateActor(sim, ent, delta); } break;
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
            { ZE_ASSERT(0, "Unknown Ent Tick Type"); } break;
        }

        // make sure previous positions are updated
        ent->body.previousPos = ent->body.t.pos;
    }
    sim->tick++;
    sim->time += delta;
}

#endif // SIM_ENTITY_TICKS_H
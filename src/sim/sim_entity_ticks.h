#pragma once

#include "sim.h"
/*
Tick functions shared between client and server
*/

extern "C" i32 Sim_TickSpawn(
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
        ent->body.t.scale.x = ZE_LerpF32(0.01f, halfSize->x * 2, time);
        ent->body.t.scale.y = ZE_LerpF32(50.0f, halfSize->y * 2, time);
        ent->body.t.scale.z = ZE_LerpF32(0.01f, halfSize->z * 2, time);
    }
    return ZE_ERROR_NONE;
}

extern "C" void SimEnt_TickSeeker(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
	
}

extern "C" void SimEnt_TickDart(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
	
}

extern "C" void SimEnt_TickBouncer(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
	
}

extern "C" void SimEnt_TickWanderer(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
	
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
    
    f32 stepSpeed = ent->movement.speed * dt;

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
    forward.x = (sinf(radiansForward) * stepSpeed) * dir.z;
    forward.y = 0;
    forward.z = (cosf(radiansForward) * stepSpeed) * dir.z;

    Vec3 left;
    left.x = (sinf(radiansLeft) * stepSpeed) * dir.x;
    left.y = 0;
    left.z = (cosf(radiansLeft) * stepSpeed) * dir.x;

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
		move.x = ent->movement.velocity.x * dt;
	    move.z = ent->movement.velocity.z * dt;
	}
    else
    {
        // normal walk
        move.x = forward.x + left.x + up.x;
        move.z = forward.z + left.z + up.z;
    }
    
    Transform* t = &ent->body.t;
    ent->body.previousPos = t->pos;
    t->pos.x += move.x;
    t->pos.y += move.y;
    t->pos.z += move.z;

	Sim_BoundaryBounce(ent, &sim->boundaryMin, &sim->boundaryMax);
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

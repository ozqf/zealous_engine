#ifndef SIM_ENT_ACTOR_H
#define SIM_ENT_ACTOR_H

#include "sim_internal.h"

//////////////////////////////////////////////
// Actor look + movement
//////////////////////////////////////////////

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

    // ground check
    i32 bGrounded = Sim_GroundCheck(sim, ent);
    if (bGrounded)
    {
        ent->movement.flags |= SIM_ENT_MOVE_BIT_GROUNDED;
        move.y = 0;
        
    }
    else
    {
        ent->movement.flags &= ~SIM_ENT_MOVE_BIT_GROUNDED;
        move.y += sim->info.gravity.y * dt;
    }
    
    Transform* t = &ent->body.t;
    // record previous position
    ent->body.previousPos = t->pos;
    // Apply
	Sim_BoundaryBounce(ent, &sim->info.boundaryMin, &sim->info.boundaryMax);
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
	Sim_BoundaryBounce(ent, &sim->info.boundaryMin, &sim->info.boundaryMax);

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
	Sim_BoundaryBounce(ent, &sim->info.boundaryMin, &sim->info.boundaryMax);
}
#endif

/**
 * Input is optional to override the input attached to ent for client prediction
 */
internal void SimEnt_StepActorMovement(
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

internal void SimEnt_FireAttack(
	SimScene* sim,
    SimEntity* ent,
    Vec3 dir, // not used!
    u8 factoryType, // not used!
    i32 numProjectiles)
{
	if (numProjectiles <= 0) { numProjectiles = 1; }

    SimEvent_BulkSpawn event = {};
    Transform t = ent->body.t;
    Sim_SetBulkSpawn(
        &event,
        Sim_ReserveEntitySerials(sim, 0, numProjectiles),
        ent->id.serial,
        t,
        sim->info.tick,
        factoryType,
        ent->teamId,
        SIM_PATTERN_3D_CONE,
        (u8)numProjectiles,
        COM_STDRandU8(),
        0,
        0.1f
    );

	ZCmd_Write(&event.header, &sim->data.outputBuf->cursor);
}

//////////////////////////////////////////////
// Actor attacks
//////////////////////////////////////////////
internal void SimEnt_TickActorAttack(
    SimScene* sim, SimEntity* ent, timeFloat dt, i32 bIsServer)
{
    SimActorInput input = ent->input;
	
    if (input.HasBitToggledOff(ACTOR_INPUT_SLOT_1))
    {
        printf("Select slot 1\n");
        ent->inventory.pendingIndex = 0;
    }
    if (input.HasBitToggledOff(ACTOR_INPUT_SLOT_2))
    {
        printf("Select slot 2\n");
        ent->inventory.pendingIndex = 1;
    }
    if (input.HasBitToggledOff(ACTOR_INPUT_SLOT_3))
    {
        printf("Select slot 3\n");
        ent->inventory.pendingIndex = 2;
    }
    if (input.HasBitToggledOff(ACTOR_INPUT_SLOT_4))
    {
        printf("Select slot 4\n");
        ent->inventory.pendingIndex = 3;
    }

	if (ent->attackTick <= 0)
	{
        if (ent->inventory.pendingIndex != ent->inventory.index)
        {
            ent->inventory.index = ent->inventory.pendingIndex;
            ent->inventory.pendingIndex = ent->inventory.index;
        }
        if (input.buttons & ACTOR_INPUT_ATTACK)
        {
            i32 index = ent->inventory.index;
            SimInventoryItem* item = SVI_GetItem(index);

            // shoot
            ent->attackTick = item->duration;
            Vec3 forward = ent->body.t.rotation.zAxis;
            // flip
            forward.x = -forward.x;
            forward.y = -forward.y;
            forward.z = -forward.z;
            if (item->eventType == SIM_ITEM_EVENT_TYPE_PROJECTILE)
            {
                SimEnt_FireAttack(sim, ent, forward, item->factoryType, item->eventCount);
            }
			else
            {
                printf("Weapon has unknown event type %d\n", item->eventType);
            }
            
        }
	}
	else
	{
		ent->attackTick -= dt;
	}
}

internal void SimEnt_TickActor(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer)
{
    SimEnt_StepActorMovement(sim, ent, &ent->input, deltaTime);
	if (bIsServer)
	{
		SimEnt_TickActorAttack(sim, ent, deltaTime, bIsServer);
	}
}

#endif //SIM_ENT_ACTOR_H
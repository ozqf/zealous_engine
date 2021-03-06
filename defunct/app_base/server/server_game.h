#pragma once

#include "server_internal.h"
#include <math.h>

internal void SVG_ReplicateSpawn(
    SimScene* sim, SimEvent_BulkSpawn* event,
    i32 bSyncPosition, f32 priority)
{
    // Replicate!
    S2C_BulkSpawn cmd = {};
    Cmd_InitBulkSpawn(&cmd, event, sim->tick);
	SVU_EnqueueCommandForAllUsers(&g_users, &cmd.header);
    if (bSyncPosition)
    {
        SVU_AddBulkEntityLinksForAllUsers(
            &g_users,
            event->base.firstSerial,
            event->patternDef.numItems,
            priority);
    }
}

// Returns target if found
internal SimEntity* SVG_FindAndValidateTarget(
    SimScene* sim, SimEntity* ent)
{
    /*
    > if no target Id => get a target Id and retrieve Ent
    > if target Id => retrieve Ent
    > Validate Target Ent. 
    */
   SimEntity* target = NULL;
   if (ent->relationships.targetId.serial == SIM_ENT_NULL_SERIAL)
   {
       // Try to find a new target
       target = Sim_FindTargetForEnt(sim, ent);
       if (target == NULL) { return NULL; }
       ent->relationships.targetId = target->id;
       return target;
   }
   else
   {
       // Check current target and clear if invalidated
       target = Sim_GetEntityByIndex(sim, ent->relationships.targetId.slot);
       // check target is still okay
       if (Sim_IsEntInPlay(target) == NO)
       {
           ent->relationships.targetId = {};
           return NULL;
       }
       return target;
   }
}

////////////////////////////////////////////////////////
// Entity Death
////////////////////////////////////////////////////////
internal void SVG_HandleEntityDeath(
    SimScene* sim, SimEntity* victim, SimEntity* attacker, i32 style, i32 deathIsDeterministic)
{
	APP_LOG(128, "SV Remove ent %d\n", victim->id.serial);
    SimEntity* parent = Sim_GetEntityBySerial(
        sim, victim->relationships.parentId.serial);
    // if (victim->factoryType == SIM_FACTORY_TYPE_SEEKER)
    // {
    //     printf("SVG - kill seeker\n");
    // }
    if (parent != NULL)
    {
        parent->relationships.liveChildren--;
    }
    SVU_RemoveEntityForAllUsers(victim, &g_users, victim->id.serial);
    // deterministic deaths will occur naturally on the client without server info
    #if 0
    if (!deathIsDeterministi
    {
        SVU_RemoveEntityForAllUsers(victim, &g_users, victim->id.serial);

        // Alter replication of event depending on relationship
        // to a user
        #if 0
        if (victim->flags & SIM_ENT_FLAG_POSITION_SYNC)
        {
            SVU_RemoveEntityForAllUsers(&g_users, victim->id.serial);
        }
        else
        {
            S2C_RemoveEntity cmd = {};
            Cmd_InitRemoveEntity(&cmd, g_ticks, 0, victim->id.serial);
            SVU_EnqueueCommandForAllUsers(&g_users, &cmd.header);
        }
        #endif
    }
    #endif
	// Remove Ent AFTER command as sim may
	// clear entity details immediately
	Sim_RemoveEntity(sim, victim->id.serial);
}

internal void SVG_SpawnLineSegment(SimScene* sim, Vec3 origin, Vec3 dest)
{
    SimEvent_Spawn def = {};
    def.factoryType = SIM_FACTORY_TYPE_LINE_TRACE;
    def.serial = Sim_ReserveEntitySerial(sim, 1);
    def.isLocal = 1;
    def.pos = origin;
    def.destination = dest;
    Sim_RestoreEntity(sim, &def);
}

#define SVG_DEFINE_ENT_UPDATE(entityTypeName) internal void \
    SVG_Update##entityTypeName##(SimScene* sim, SimEntity* ent, timeFloat deltaTime)

SVG_DEFINE_ENT_UPDATE(LineTrace)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        Sim_RemoveEntity(sim, ent->id.serial);
    }
}

//////////////////////////////////////////////////////
// Spawner
//////////////////////////////////////////////////////
SVG_DEFINE_ENT_UPDATE(Spawner)
{
    i32 spawnSpaces = ent->relationships.maxLiveChildren - 
        ent->relationships.liveChildren;
    if (spawnSpaces < ent->relationships.childSpawnCount)
    { return; }

    if (sim->tick >= ent->timing.nextThink)
    {
        ent->timing.lastThink = ent->timing.nextThink;
        ent->timing.nextThink += App_CalcTickInterval(2);
        // think
        // Spawn projectiles
        SimEvent_BulkSpawn event = {};
        /*
        event.factoryType = ent->relationships.childFactoryType;
        event.base.firstSerial = Sim_ReserveEntitySerials(
            sim, 0, ent->relationships.childSpawnCount);
        event.base.pos = ent->body.t.pos;
        event.patternDef.numItems = ent->relationships.childSpawnCount;
        event.patternDef.patternId = SIM_PATTERN_FLAT_RADIAL;
        event.patternDef.radius = 10.0f;
        event.base.seedIndex = COM_STDRandU8();
        event.base.forward = { 0, 0, 1 };
        // frame the event occurred on is recorded
        event.base.tick = sim->tick;
        event.base.sourceSerial = ent->id.serial;
        */
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

        i32 flags;
        f32 priority;
		Sim_ExecuteBulkSpawn(
			sim, &event, 0, &flags, &priority);

        ent->relationships.liveChildren += 
            ent->relationships.childSpawnCount;
        
        // Replicate!
        SVG_ReplicateSpawn(
            sim, &event, flags & SIM_ENT_FLAG_POSITION_SYNC, priority);
        #if 0
        S2C_BulkSpawn prj = {};
        Cmd_InitBulkSpawn(&prj, &event, g_ticks, 0);
		SVU_EnqueueCommandForAllUsers(&g_users, &prj.header);
        if (flags & SIM_ENT_FLAG_POSITION_SYNC)
        {
            SVU_AddBulkEntityLinksForAllUsers(
                &g_users, event.base.firstSerial, event.patternDef.numItems, priority);
        }
        #endif
    }
}

//////////////////////////////////////////////////////
// Projectiles
//////////////////////////////////////////////////////
internal i32 SVG_StepProjectile(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    Vec3 frameOrigin = ent->body.t.pos;
    Vec3 frameMove;
    frameMove.x = ent->movement.velocity.x * (f32)deltaTime;
    frameMove.y = ent->movement.velocity.y * (f32)deltaTime;
    frameMove.z = ent->movement.velocity.z * (f32)deltaTime;
    Vec3 frameDest;
    frameDest.x = frameOrigin.x + frameMove.x;
    frameDest.y = frameOrigin.y + frameMove.y;
    frameDest.z = frameOrigin.z + frameMove.z;

    // by raycast
    const i32 max_overlaps = 16;
    SimRaycastResult results[max_overlaps];
    i32 overlaps = 0;
    overlaps = Sim_FindByRaycast(
        sim, frameOrigin, frameDest, {}, ent->id.serial, results, max_overlaps);
    i32 killed = NO;
    //printf("SIM prj hits %d\n", overlaps);
    for (i32 i = 0; i < overlaps; ++i)
    {
        SimEntity* victim = results[i].ent;
        if (Sim_IsEntTargetable(victim) == NO) { continue; }
        ZE_ASSERT(victim->id.serial, "SV overlap victim serial is 0")
        
        if ((victim->flags & SIM_ENT_FLAG_INVULNERABLE) == 0)
        {
            // Hurt/kill victim
            victim->life.health -= ent->touchDamage;
            if (victim->life.health <= 0)
            {
                SVG_HandleEntityDeath(sim, victim, ent, 0, 0);
            }
        }
        frameDest = results[i].hitPos;
        killed = YES;

        break;
    }
    
    // place projectile at end of move
    ent->body.t.pos = frameDest;
    
    if (killed == YES)
    {
        /*printf("PRJ real birth tick vs sim tick: %d, %d\n", (i32)ent->timing.realBirthTick, (i32)sim->tick);
        if (ent->timing.realBirthTick == sim->tick)
        {
            printf("\tPRJ died on birth tick!\n");
        }*/
        SVG_HandleEntityDeath(sim, ent, NULL, 0, 0);
        return 0;
    }
    
    // Timeout
	if (sim->tick >= ent->timing.nextThink)
	{
        SVG_HandleEntityDeath(sim, ent, NULL, 0, 1);
        return 0;
	}
    return 1;
}

SVG_DEFINE_ENT_UPDATE(Projectile)
{
    while(ent->timing.fastForwardTicks > 0)
    {
        ent->timing.fastForwardTicks--;
        if (!SVG_StepProjectile(sim, ent, deltaTime))
        {
            return;
        }
    }
	SVG_StepProjectile(sim, ent, deltaTime);
}

internal void SVG_FireActorAttack(
    SimScene* sim,
    SimEntity* ent,
    Vec3* dir,
    i32 prjType,
    i32 numProjectiles)
{
    /*
    > identify if player...
        ...If not just spawn with no faff, otherwise:
    > Calculate the frame command came from - how?
         eg if server is on 12203 and CL on 11994:
         Client is running 5 ticks of time + 4 ticks of jitter
            behind server
         > Client has simulated forward 9 frames to match it's server
            position at this moment
    > Retrieve entity position at that point
    > Spawn projectiles and enqueue replication command
    > fast forward projectiles to present
    */

    /*
	500ms round trip lag == (250ms / 16ms) or 15.625 ticks since player pressed button
		plus a few frames for player's jitter.
    Event occurred at client tick. Therefore server must fast-forward the projectile
        by (server tick) - (client tick) ticks.
	eg server tick 100, client event from tick 80:
	> Server must fast-forward 20 ticks.
	> All clients are informed the event occurred on tick 80 and will also fast-forward
	
	If Prj moves at 15 units a second, at 60fps, 0.25 per frame, 20 frames == 5 units
	
    */
    //printf("SVG Shoot %.3f, %.3f\n", dir->x, dir->z);
    const i32 verbose = 0;

    i32 fastForwardTicks = 0;
    User* u = User_FindByAvatarSerial(&g_users, ent->id.serial);
    if (u && g_lagCompensateProjectiles)
    {
        // calculate fast-forward ticks
        // TODO: Tune these and figure out which of these is best
        i32 ticksEllapsed = 0;
        i32 diff = 0;

        // 1: By estimating current lag
        // Works well until jitter is introduced, and then becomes
        // inaccurate (though test jitter is excessive...)
        timeFloat time = u->ping;// * 0.5f;
        ticksEllapsed = (i32)(time / App_GetSimFrameInterval());
        //ticksEllapsed += APP_DEFAULT_JITTER_TICKS;
        #if 0
        fastForwardTicks = ticksEllapsed;
        #endif
 
        // 2: By diffing from client's last stated frame
        // Works well but is this exploitable...? Trust client's tick value
        diff = sim->tick - u->latestServerTick;
        #if 1
        fastForwardTicks = diff;
        #endif
        if (verbose)
        {
            printf(
                "Prj ping %.3f, ticksEllapsed - %d ticks (diff %d)\n",
                u->ping, ticksEllapsed, diff);
        }
    }

    // Declare when the event took place:
    i32 eventTick = sim->tick - fastForwardTicks;
    if (verbose)
    {
        printf("SV CurTick %d eventTick %d fastforward %d\n",
            sim->tick, eventTick, fastForwardTicks);
    }

    //i32 numProjectiles = ent->relationships.childSpawnCount;
    if (numProjectiles <= 0) { numProjectiles = 1; }
    
    SimEvent_BulkSpawn event = {};
    Transform t = ent->body.t;
    Sim_SetBulkSpawn(
        &event,
        Sim_ReserveEntitySerials(sim, 0, numProjectiles),
        ent->id.serial,
        t,
        sim->tick,
        SIM_FACTORY_TYPE_PROJ_PLAYER,
        SIM_PATTERN_3D_CONE,
        (u8)numProjectiles,
        COM_STDRandU8(),
        0,
        0.1f
    );

    i32 flags;
    f32 priority;
    Sim_ExecuteBulkSpawn(sim, &event, fastForwardTicks, &flags, &priority);

    // Replicate
    S2C_BulkSpawn prj = {};
    Cmd_Prepare(&prj.header, eventTick);
    prj.def = event;
    prj.header.type = CMD_TYPE_S2C_BULK_SPAWN;
    prj.header.size = sizeof(prj);
    SVU_EnqueueCommandForAllUsers(&g_users, &prj.header);

    if (flags & SIM_ENT_FLAG_POSITION_SYNC)
    {
        SVU_AddBulkEntityLinksForAllUsers(
            &g_users,
            event.base.firstSerial,
            event.patternDef.numItems, priority);
    }
    
    /* Debug - Create a line trace */
    Vec3 origin = ent->body.t.pos;
    Vec3 dest {};
    dest.x = (origin.x + (dir->x * 10));
    dest.y = (origin.y + (dir->y * 10));
    dest.z = (origin.z + (dir->z * 10));
    SVG_SpawnLineSegment(sim, origin, dest);
}

internal i32 SVG_CheckInputJustOn(SimActorInput input, i32 bit)
{
    if (input.buttons & bit &&
        (input.prevButtons & bit) == 0)
    {
        return YES;
    }
    return NO;
}

//////////////////////////////////////////////////////
// Update Actor Attack Input
//////////////////////////////////////////////////////
internal void SVG_UpdateActorAttackInput(SimScene* sim, SimEntity* ent, f32 dt)
{
    SimActorInput input = ent->input;
    if (SVG_CheckInputJustOn(input, ACTOR_INPUT_SLOT_1))
    {
        printf("Select slot 1\n");
        ent->inventory.pendingIndex = 0;
    }
    if (SVG_CheckInputJustOn(input, ACTOR_INPUT_SLOT_2))
    {
        printf("Select slot 2\n");
        ent->inventory.pendingIndex = 1;
    }
    if (SVG_CheckInputJustOn(input, ACTOR_INPUT_SLOT_3))
    {
        printf("Select slot 3\n");
        ent->inventory.pendingIndex = 2;
    }
    if (SVG_CheckInputJustOn(input, ACTOR_INPUT_SLOT_4))
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
			SVG_FireActorAttack(sim, ent, &forward, item->eventType, item->eventCount);
        }
        #if 0
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
			SVG_FireActorAttack(sim, ent, &shoot);
		}
        #endif
	}
	else
	{
		ent->attackTick -= dt;
	}
}

SVG_DEFINE_ENT_UPDATE(Actor)
{
    f32 dt = (f32)deltaTime;
    // printf("SV Actor look %.3f, %.3f\n",
    //     ent->input.degrees.x, ent->input.degrees.y
    // );
    //printf("SV Step actor %.3f, %.3f, %.3f\n",
    //    ent->body.t.pos.x, ent->body.t.pos.y, ent->body.t.pos.z
    //);
    SimEnt_StepActorMovement(sim, ent, NULL, dt);

    // Attacks - server side only atm
	switch (ent->movement.moveMode)
	{
		case 0: // Walking
		{
			//SVG_UpdateActorWalk(sim, ent, dt);
			SVG_UpdateActorAttackInput(sim, ent, dt);
		} break;
		
		case 1: // Dodging
		{
			//SVG_UpdateActorEvade(sim, ent, dt);
		} break;
		
		default:
		{
			
		} break;
	}
}

SVG_DEFINE_ENT_UPDATE(Bot)
{
    u32 buttons = 0;
    buttons |= ACTOR_INPUT_SHOOT_LEFT;
    //buttons |= ACTOR_INPUT_SHOOT_UP;
    ent->input.buttons = buttons;
    SVG_UpdateActor(sim, ent, deltaTime);
}

internal void SVG_TickEntity(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    #if 0
    #ifdef SIM_QUANTISE_SYNC
    // Quantise physical properties
    if (ent->flags & SIM_ENT_FLAG_POSITION_SYNC)
    {
        // quantise position, velocity and rotation
        COM_QuantiseVec3(&ent->body.t.pos, &sim->quantise.pos);
        COM_QuantiseVec3(&ent->movement.velocity, &sim->quantise.vel);
    }
    #endif
    #endif
	const i32 bIsServer = YES;
    switch (ent->tickType)
    {
		case SIM_TICK_TYPE_PROJECTILE:
        { SVG_UpdateProjectile(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SEEKER:
        { SimEnt_TickSeeker(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_SEEKER_FLYING:
		{ SimEnt_TickSeekerFlying(sim, ent, deltaTime, bIsServer); } break;
		case SIM_TICK_TYPE_WANDERER:
        { SimEnt_TickWanderer(sim, ent, deltaTime, bIsServer); break; }
        case SIM_TICK_TYPE_BOUNCER:
        { SimEnt_TickBouncer(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_DART:
        { SimEnt_TickDart(sim, ent, deltaTime, bIsServer); } break;
		case SIM_TICK_TYPE_ACTOR:
        { SVG_UpdateActor(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_BOT:
        { SVG_UpdateBot(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SPAWNER:
        { SVG_UpdateSpawner(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_LINE_TRACE:
        { SVG_UpdateLineTrace(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SPAWN:
        { SimEnt_TickSpawnAnimation(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_WORLD: { } break;
        case SIM_TICK_TYPE_NONE: { } break;
        default:
        { ZE_ASSERT(0, "Unknown Ent Tick Type"); } break;
    }
}

internal void SVG_TickSim(SimScene* sim, timeFloat deltaTime)
{
    AppTimer timer(APP_STAT_SV_SIM, g_sim.tick);
	sim->timeInAABBSearch = 0;
    for (i32 i = 0; i < g_sim.maxEnts; ++i)
    {
        SimEntity* ent = &g_sim.ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }

        SVG_TickEntity(sim, ent, deltaTime);
        // make sure previous positions are updated
        ent->body.previousPos = ent->body.t.pos;
    }
    sim->tick++;
    sim->time += deltaTime;

    #ifdef SIM_USE_PHYSICS_ENGINE
    ZEByteBuffer output = PhysExt_Step(sim->world, deltaTime);
    u8* read = output.start;
    u8* end = output.ptrEnd;
    while (read < end)
    {
        PhysEv_Header* h = (PhysEv_Header*)read;
        ErrorCode err = Phys_ValidateEvent(h);
        ZE_ASSERT(err == ZE_ERROR_NONE, "Error reading phys event")
        read += h->size;
        switch (h->type)
        {
            case TransformUpdate:
            {
                PhysEV_TransformUpdate* t = (PhysEV_TransformUpdate*)h;
                SimEntity* ent = Sim_GetEntityBySerial(&g_sim, t->ownerId);
                if (ent != NULL)
                {
                    Vec3 pos =
                    {
                        t->matrix[M4x4_W0],
                        t->matrix[M4x4_W1],
                        t->matrix[M4x4_W2]
                    };
                    ent->movement.velocity.x = t->vel[0];
                    ent->movement.velocity.y = t->vel[1];
                    ent->movement.velocity.z = t->vel[2];
                    ent->body.t.pos = pos;
                    /*if (ent->id.serial > 0)
                    {
                        printf("SVG Phys update shape %d (ent type %d) to %.3f, %.3f, %.3f\n",
                            t->ownerId, ent->factoryType, pos.x, pos.y, pos.z);
                    }*/
                    
                }
                else
                {
                    printf("SVG Phys no ent for shape %d\n", t->ownerId);
                }
                
                
            } break;
            case RaycastDebug:
            {

            } break;
            case OverlapStarted:
            {
                PhysEv_Collision* col = (PhysEv_Collision*)h;
                printf("SVG Phys collision! %d vs %d\n", col->a.externalId, col->b.externalId);
            } break;
            case OverlapInProgress:
            {

            } break;
            case OverlapEnded:
            {

            } break;
            default:
            {
                ZE_ASSERT(0, "Unknown physics event type")
            } break;
        }
    }
    #endif
}

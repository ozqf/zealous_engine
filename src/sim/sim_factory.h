#pragma once

/*
> Manages SimEntity pool
> Performs new entity configuration
*/
#include "sim.h"

////////////////////////////////////////////////////////////////////
// Entity assignment
////////////////////////////////////////////////////////////////////
internal SimEntity* Sim_FindEntityBySerialNumber(
    SimScene* scene, i32 serialNumber)
{
    for (i32 j = 0; j < scene->maxEnts; ++j)
    {
        SimEntity* ent = &scene->ents[j];
        if (ent->id.serial == serialNumber)
        {
            return ent;
        }
    }
    return NULL;
}

internal i32 Sim_FreeEntityBySerial(SimScene* scene, i32 serial)
{
	for (i32 i = 0; i < scene->maxEnts; ++i)
	{
		SimEntity* ent = &scene->ents[i];
		
		if (ent->id.serial != serial) { continue; }
		
		// free slot
		ent->status = SIM_ENT_STATUS_FREE;
		
		return ZE_ERROR_NONE;
	}
	return ZE_ERROR_NOT_FOUND;
}

internal i32 Sim_FindFreeSlot(SimScene* scene, i32 forLocalEnt)
{
    i32 halfMax = scene->maxEnts / 2;
    i32 i = forLocalEnt ? halfMax : 0;
    i32 l = forLocalEnt ? scene->maxEnts : halfMax;
    for (; i < l; ++i)
    {
        SimEntity* ent = &scene->ents[i];
        if (ent->status != SIM_ENT_STATUS_FREE) { continue; }
        return i;
    }
    return -1;
}

internal SimEntity* Sim_GetFreeReplicatedEntity(
    SimScene* scene, i32 newSerial)
{
    SimEntity* ent = NULL;
    i32 slotIndex = -1;
    slotIndex = Sim_FindFreeSlot(scene, 0);
    if (slotIndex <= -1 ) { return NULL; }

    // config
    ent = &scene->ents[slotIndex];
    ent->status = SIM_ENT_STATUS_IN_USE;
    ent->id.slot.index = (u16)slotIndex;
	ent->id.serial = newSerial;
    // For clients, keep track of the latest entity spawned.
    if (newSerial > scene->highestAssignedSequence)
    {
        scene->highestAssignedSequence = newSerial;
    }
    if (scene->bVerbose)
    {
        APP_LOG(64,
            "SIM assigned replicated ent serial %d (slot %d/%d)\n",
            ent->id.serial, ent->id.slot.iteration, ent->id.slot.index);
    }
	
    ent->isLocal = 0;
    return ent;
}

internal SimEntity* Sim_GetFreeLocalEntity(
    SimScene* scene, i32 newSerial)
{
    SimEntity* ent = NULL;
    i32 slotIndex = -1;
    slotIndex = Sim_FindFreeSlot(scene, 1);
    if (slotIndex <= -1 ) { return NULL; }

    // config
    ent = &scene->ents[slotIndex];
    ent->status = SIM_ENT_STATUS_IN_USE;
    ent->id.slot.index = (u16)slotIndex;
	ent->id.serial = newSerial;
    if (scene->bVerbose)
    {
        APP_LOG(64, "SIM assigned local ent serial %d (slot %d/%d)\n",
            ent->id.serial, ent->id.slot.iteration, ent->id.slot.index);
    }
    ent->isLocal = 1;
    return ent;
}

internal i32 Sim_RecycleEntity(
    SimScene* sim, i32 entitySerialNumber)
{
    SimEntity* ent = Sim_FindEntityBySerialNumber(
        sim, entitySerialNumber);
    if (ent)
    {
        // Check for and inform parent
        i32 parentSerial = ent->relationships.parentId.serial;
        if (parentSerial != SIM_ENT_NULL_SERIAL)
        {
            SimEntity* parent = Sim_GetEntityBySerial(sim, parentSerial);
            if (parent == NULL)
            {
                printf("SIM no parent %d for recycling ent %d\n", parentSerial, ent->id.serial);
            }
            else
            {
                parent->relationships.liveChildren--;
            }
        }

        // remove
        SimEntIndex slot = ent->id.slot;
        #if 0
        if (sim->bVerbose)
        {
            APP_LOG(64, "SIM Removing ent %d (slot %d/%d)\n",
                entitySerialNumber, slot.iteration, slot.index);
        }
        #endif
        #ifdef SIM_USE_PHYSICS_ENGINE
        if (ent->shape.handleId > 0)
        {
            PhysCmd_RemoveShape(sim->world, ent->shape.handleId);
            ent->shape = {};
        }
        #endif
        u16 iteration = ent->id.slot.iteration + 1;
        *ent = {};
        ent->status = SIM_ENT_STATUS_FREE;
        ent->id.slot.iteration = iteration;
        return ZE_ERROR_NONE;
    }
    else
    {
        if (sim->bVerbose)
        {
            APP_LOG(64, "SIM Found no ent %d to remove\n",
                entitySerialNumber);
        }
        return ZE_ERROR_BAD_ARGUMENT;
    }
}

/**
 * Marks an entity as removed so that it is not involved in any further logic.
 * the actual removal will occur on the following frame.
 */
internal i32 Sim_MarkEntityAsRemoved(SimScene* sim, i32 serialNumber)
{
	ZE_ASSERT(serialNumber != SIM_ENT_NULL_SERIAL,
        "Removing entity with null serial")
    if (serialNumber == SIM_ENT_NULL_SERIAL)
    {
        printf("SIM - mark for cull serial is empty\n");
        return ZE_ERROR_BAD_ARGUMENT;
    }
    SimEntity* ent = Sim_GetEntityBySerial(sim, serialNumber);
    if (ent == NULL)
    {
        printf("SIM - no ent %d to mark for cull\n", serialNumber);
        return ZE_ERROR_BAD_ARGUMENT;
    }
    ent->status = SIM_ENT_STATUS_CULL;
    return ZE_ERROR_NONE;
    //return Sim_RecycleEntity(sim, serialNumber);
}

internal void Sim_WriteRemoveEntity(
    SimScene* sim,
    SimEntity* victim,
    SimEntity* attacker,
    i32 style,
    Vec3 dir,
    i32 deathIsDeterministic)
{
    SimEvent_RemoveEnt ev = {};
    ev.header.type = SIM_CMD_TYPE_REMOVE_ENTITY;
    ev.header.size = sizeof(SimEvent_RemoveEnt);
    ev.header.sentinel = ZCMD_SENTINEL;
    ev.entityId = victim->id.serial;
    ev.style = style;
    ev.dir = dir;
    ZCmd_Write(&ev.header, &sim->outputBuf->cursor);

    Sim_MarkEntityAsRemoved(sim, victim->id.serial);
}

////////////////////////////////////////////////////////////////////
// Entity initialisation
////////////////////////////////////////////////////////////////////
internal i32 Sim_InitActor(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    printf("SIM Create actor, scale %.3f, %.3f, %.3f\n",
        def->scale.x, def->scale.y, def->scale.z
    );
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityBody(ent, { 1, 2, 1 });
    Sim_SetEntityStats(ent, ACTOR_BASE_SPEED, 1, 0.05f);
    Sim_SetEntityDisplay_Mesh(ent,
        { 0, 1, 0, 1 },
        { 0, 1, 0, 1 },
        ZRDB_MESH_NAME_CUBE,
		ZRDB_DEFAULT_DIFFUSE_MAT_NAME,
        SIM_DEATH_GFX_BULLET_IMPACT);
    ent->tickType = SIM_TICK_TYPE_ACTOR;
    ent->coreTickType = SIM_TICK_TYPE_ACTOR;
    ent->attackTime = 0.5f;
    ent->relationships.childSpawnCount = SIM_PLAYER_SHOTGUN_PELLETS;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitBot(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityStats(ent, 6.5f, 1, 0.05f);
    Sim_SetEntityDisplay_Mesh(ent,
        { 0, 0.6f, 0, 1 },
        { 0, 0.6f, 0, 1 },
        ZRDB_MESH_NAME_CUBE,
		ZRDB_DEFAULT_DIFFUSE_MAT_NAME,
        SIM_DEATH_GFX_BULLET_IMPACT);
    ent->tickType = SIM_TICK_TYPE_BOT;
    ent->coreTickType = SIM_TICK_TYPE_BOT;
    //ent->flags = SIM_ENT_FLAG_POSITION_SYNC;
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitSpawner(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    ent->tickType = SIM_TICK_TYPE_SPAWNER;
    ent->coreTickType = SIM_TICK_TYPE_SPAWNER;
    ent->relationships.childSpawnCount = def->numChildren;
    ent->relationships.maxLiveChildren = def->numChildren;
    ent->relationships.totalChildren = def->numChildren;
    ent->relationships.patternType = def->patternType;
    return ZE_ERROR_NONE;
}

///////////////////////////////////////////////////////
// WORLD
///////////////////////////////////////////////////////
internal i32 Sim_InitWorldVolume(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    APP_PRINT(256, "SIM Spawning world volume at %.3f, %.3f, %.3f\n",
        def->pos.x, def->pos.y, def->pos.z);
    ent->tickType = SIM_TICK_TYPE_WORLD;
    ent->coreTickType = SIM_TICK_TYPE_WORLD;
    ent->flags = SIM_ENT_FLAG_SHOOTABLE | SIM_ENT_FLAG_INVULNERABLE;
    // world volumes can't move (yet!)
    ent->movement.velocity = {};
    Sim_SetEntityDisplay_Mesh(ent,
        { 0.2f, 0.2f, 0.2f, 1 },
        { 0.2f, 0.2f, 0.2f, 1 },
        ZRDB_MESH_NAME_CUBE,
		ZRDB_MAT_NAME_WORLD,
		//"app_mesh",
		//"grid",
        SIM_DEATH_GFX_NONE);
    #ifdef SIM_USE_PHYSICS_ENGINE
    ent->shape.SetAsBox(def->pos, def->scale, ZCOLLIDER_FLAG_STATIC, SIM_LAYER_WORLD, SIM_LAYER_WORLD, 0);
    PhysCmd_CreateShape(scene->world, &ent->shape, ent->id.serial);
    #endif
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitPointLight(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* def)
{
    Vec3 settings = def->scale;
    def->scale = { 1, 1, 1 };
    Sim_SetEntityBase(ent, def);
    ent->tickType = SIM_TICK_TYPE_NONE;
    ent->coreTickType = SIM_TICK_TYPE_NONE;
    Colour colour;
    colour.r = def->pointLight.colour.x;
    colour.g = def->pointLight.colour.y;
    colour.b = def->pointLight.colour.z;
    colour.a = 1;
    ent->display.data.SetAsPointLight(colour, def->pointLight.multiplier, def->pointLight.range);
    ent->lightType = 1;
    //Sim_SetEntityDisplay_Mesh(ent,
    //    colour,
    //    // TODO: Encoding light settings in the light's second channel? HACK! At least use a union...
    //    { def->pointLight.multiplier, def->pointLight.range, 0, 1 },
    //    "Cube",
	//	"Default",
    //    SIM_DEATH_GFX_NONE);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitDirectLight(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* def)
{
    Vec3 settings = def->scale;
    def->scale = { 1, 1, 1 };
    Sim_SetEntityBase(ent, def);
    ent->tickType = SIM_TICK_TYPE_NONE;
    ent->coreTickType = SIM_TICK_TYPE_NONE;
    Colour colour;
    colour.r = def->pointLight.colour.x;
    colour.g = def->pointLight.colour.y;
    colour.b = def->pointLight.colour.z;
    colour.a = 1;
    // Sim_SetEntityDisplay_Mesh(ent,
    //     colour,
    //     // TODO: Encoding light settings in the light's second channel? HACK! At least use a union...
    //     { def->pointLight.multiplier, def->pointLight.range, 0, 1 },
    //     "Cube",
    //     "World",
    //     SIM_DEATH_GFX_NONE);
    ent->display.data.SetAsDirectLight(
        colour, def->pointLight.multiplier, def->pointLight.range);
    f32* rot = ent->body.t.rotation.cells;
    M3x3_SetToIdentity(rot);
    M3x3_RotateX(rot, def->pitchDegrees * DEG2RAD);
    M3x3_RotateY(rot, def->yawDegrees * DEG2RAD);
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitLineTrace(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
	//printf("SIM Create line trace\n");
    Sim_SetEntityBase(ent, def);
    ent->tickType = SIM_TICK_TYPE_LINE_TRACE;
    ent->coreTickType = SIM_TICK_TYPE_LINE_TRACE;
    ent->timing.nextThink = Sim_CalcThinkTick(scene, 2);
    return ZE_ERROR_NONE;
}

///////////////////////////////////////////////////////
// GFX
///////////////////////////////////////////////////////
internal i32 Sim_InitExplosion(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    ent->tickType = SIM_TICK_TYPE_EXPLOSION;
    ent->coreTickType = SIM_TICK_TYPE_EXPLOSION;
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        ZRDB_MESH_NAME_CUBE,
		ZRDB_DEFAULT_DIFFUSE_MAT_NAME,
        SIM_DEATH_GFX_NONE);
    ent->timing.nextThink = Sim_CalcThinkTick(sim, 0.5f);
    ent->body.t.scale = { 2, 1, 2 };
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitBulletImpact(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    ent->tickType = SIM_TICK_TYPE_EXPLOSION;
    ent->coreTickType = SIM_TICK_TYPE_EXPLOSION;
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        ZRDB_MESH_NAME_CUBE,
		ZRDB_MAT_NAME_GFX,
        SIM_DEATH_GFX_NONE);
    ent->timing.nextThink = Sim_CalcThinkTick(sim, 0.5f);
    ent->body.t.scale = { 0.25f, 0.25f, 0.25f };
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitTargetPoint(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    ent->tickType = SIM_TICK_TYPE_TARGET_POINT;
    ent->coreTickType = SIM_TICK_TYPE_TARGET_POINT;
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        ZRDB_MESH_NAME_CUBE,
		ZRDB_MAT_NAME_LASER,
        SIM_DEATH_GFX_NONE);
    ent->timing.nextThink = Sim_CalcThinkTick(sim, 0.5f);
    ent->body.t.scale = { 0.15f, 0.15f, 0.15f };
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitPropBillboard(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        ZRDB_MESH_NAME_QUAD,
		ZRDB_MAT_NAME_WORLD,
        SIM_DEATH_GFX_NONE);
    ent->display.data.model.billboard = 1;
    ent->tickType = SIM_TICK_TYPE_NONE;
    ent->coreTickType = SIM_TICK_TYPE_NONE;
    ent->timing.nextThink = 0;
    ent->body.t.scale = { 1, 1, 1 };
    return ZE_ERROR_NONE;
}

internal i32 Sim_InitPropMesh(
    SimScene* sim, SimEntity* ent, SimEvent_Spawn* def)
{
    Sim_SetEntityBase(ent, def);
    // TODO: Storing indexes to assets doesn't work if the
    // asset isn't loaded before this point!
    // need to store the original asset name as well
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        //ZRDB_MESH_NAME_CUBE,
        //ZRDB_MESH_NAME_SPIKE,
        //ZRDB_MESH_NAME_INVERSE_CUBE,
        "quad_gen",
		//ZRDB_MAT_NAME_WORLD,
        "city",
        SIM_DEATH_GFX_NONE);
    ent->tickType = SIM_TICK_TYPE_NONE;
    ent->coreTickType = SIM_TICK_TYPE_NONE;
    ent->timing.nextThink = 0;
    ent->body.t.scale = { 1, 1, 1 };
    return ZE_ERROR_NONE;
}

///////////////////////////////////////////////////////
// Projectiles
///////////////////////////////////////////////////////
internal i32 Sim_InitProjBase(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    SimProjectileType t;
    t.speed = 10.0f;
    t.patternDef.numItems = 1;
    t.lifeTime = 1.0f;
    t.patternDef.patternId = SIM_PATTERN_NONE;
    t.scale = { 0.5f, 0.5f, 0.5f };
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        ZRDB_MESH_NAME_SPIKE,
		ZRDB_MAT_NAME_PRJ,
        SIM_DEATH_GFX_BULLET_IMPACT);
    ent->deathType = SIM_DEATH_GFX_BULLET_IMPACT;
    // must set birth tick here
    ent->timing.birthTick = def->birthTick;
    ent->touchDamage = 10;

    Sim_InitProjectile(
        scene,
        ent,
        def->pos,
        { def->pitchDegrees, def->yawDegrees, 0 },
        def->velocity,
        &t,
        def->fastForwardTicks);

    return ZE_ERROR_NONE;
}

internal i32 Sim_InitProjPrediction(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    SimProjectileType t;
    t.speed = 45.0f;
    t.patternDef.numItems = 1;
    t.lifeTime = 2.0f;
    t.patternDef.patternId = SIM_PATTERN_NONE;
    t.scale = { 0.5f, 0.5f, 0.5f };
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        ZRDB_MESH_NAME_SPIKE,
		ZRDB_MAT_NAME_PRJ,
        SIM_DEATH_GFX_BULLET_IMPACT);
    // must set birth tick here
    ent->timing.birthTick = def->birthTick;
    ent->touchDamage = 10;

    Sim_InitProjectile(
        scene,
        ent,
        def->pos,
        { def->pitchDegrees, def->yawDegrees, 0 },
        def->velocity,
        &t,
        def->fastForwardTicks);

    return ZE_ERROR_NONE;
}

internal i32 Sim_InitPlayerProjectile(
    SimScene* scene, SimEntity* ent, SimEvent_Spawn* def)
{
    SimProjectileType t;
    t.speed = SIM_PLAYER_PROJECTILE_SPEED;
    t.patternDef.numItems = 4;
    t.lifeTime = 1.5f;
    t.patternDef.patternId = SIM_PATTERN_FLAT_RADIAL;
    t.scale = { 0.15f, 0.15f, 1 };
    Sim_SetEntityDisplay_Mesh(ent,
        { 1, 1, 0, 1 },
        { 1, 1, 0, 1 },
        ZRDB_MESH_NAME_SPIKE,
		ZRDB_MAT_NAME_PRJ,
        SIM_DEATH_GFX_BULLET_IMPACT);
    // must set birth tick here
    ent->timing.birthTick = def->birthTick;
    ent->relationships.parentId.serial = def->parentSerial;

    ent->touchDamage = 10;
    ent->lightType = 1;

    Sim_InitProjectile(
        scene,
        ent,
        def->pos,
        { def->pitchDegrees, def->yawDegrees, 0 },
        def->velocity,
        &t,
        def->fastForwardTicks);
    if (ent->relationships.parentId.serial == scene->localAvatarId)
    {
        ent->display.flags |= SIM_DISPLAY_FLAG_DISABLED;
    }
    return ZE_ERROR_NONE;
}

internal SimEntity* Sim_SpawnEntity(
    SimScene* sim, SimEvent_Spawn* def)
{
    SimEntity* ent;
    if (def->isLocal)
    {
        ent = Sim_GetFreeLocalEntity(sim, def->serial);
    }
    else
    {
        ent = Sim_GetFreeReplicatedEntity(sim, def->serial);
    }
    if (!ent)
    {
        APP_PRINT(64, "SIM No Free Entity Available for Ent %d!\n",
            def->serial
        );
    }
    
    ZE_ASSERT(ent, "No free Entity")
    ent->status = SIM_ENT_STATUS_IN_USE;
    // Record factory type so we know how this entity was initialised
    ent->factoryType = def->factoryType;
    // record real birth tick, as the def's birth tick may be in the past.
    ent->timing.realBirthTick = sim->tick;

    ErrorCode err;

    switch (def->factoryType)
    {
        ////////////////////////////////////////////////////////
        // Projectiles
        case SIM_FACTORY_TYPE_PROJECTILE_BASE:
            err =  Sim_InitProjBase(sim, ent, def); break;
        case SIM_FACTORY_TYPE_PROJ_PREDICTION:
            err =  Sim_InitProjPrediction(sim, ent, def); break;
        case SIM_FACTORY_TYPE_PROJ_PLAYER:
            err =  Sim_InitPlayerProjectile(sim, ent, def); break;
        ////////////////////////////////////////////////////////
        // GFX
        case SIM_FACTORY_TYPE_BULLET_IMPACT:
            err = Sim_InitBulletImpact(sim, ent, def); break;
        case SIM_FACTORY_TYPE_EXPLOSION:
            err = Sim_InitExplosion(sim, ent, def); break;
        case SIM_FACTORY_TYPE_TARGET_POINT:
            err = Sim_InitTargetPoint(sim, ent, def); break;
        ////////////////////////////////////////////////////////
        // Mobs
        case SIM_FACTORY_TYPE_SEEKER:
            err =  Sim_InitSeeker(sim, ent, def); break;
        case SIM_FACTORY_TYPE_SEEKER_FLYING:
            err =  Sim_InitSeekerFlying(sim, ent, def); break;
        case SIM_FACTORY_TYPE_WANDERER:
            err =  Sim_InitWanderer(sim, ent, def); break;
        case SIM_FACTORY_TYPE_BOUNCER:
            err =  Sim_InitBouncer(sim, ent, def); break;
        case SIM_FACTORY_TYPE_DART:
            err = Sim_InitDart(sim, ent, def); break;
        case SIM_FACTORY_TYPE_RUBBLE:
            err = Sim_InitRubble(sim, ent, def); break;
        case SIM_FACTORY_TYPE_GRUNT:
            err = Sim_InitGrunt(sim, ent, def); break;
        ////////////////////////////////////////////////////////
        // Misc
        case SIM_FACTORY_TYPE_PROP_BILLBOARD:
            err = Sim_InitPropBillboard(sim, ent, def); break;
		case SIM_FACTORY_TYPE_PROP_MESH:
            err = Sim_InitPropMesh(sim, ent, def); break;
        case SIM_FACTORY_TYPE_ACTOR:
            err =  Sim_InitActor(sim, ent, def); break;
        case SIM_FACTORY_TYPE_BOT:
            err =  Sim_InitBot(sim, ent, def); break;
        case SIM_FACTORY_TYPE_WORLD:
            err =  Sim_InitWorldVolume(sim, ent, def); break;
        case SIM_FACTORY_TYPE_POINT_LIGHT:
            err = Sim_InitPointLight(sim, ent, def); break;
        case SIM_FACTORY_TYPE_DIRECT_LIGHT:
            err = Sim_InitDirectLight(sim, ent, def); break;
        case SIM_FACTORY_TYPE_SPAWNER:
			err = Sim_InitSpawner(sim, ent, def); break;
        case SIM_FACTORY_TYPE_LINE_TRACE:
		    err =  Sim_InitLineTrace(sim, ent, def); break;
        ////////////////////////////////////////////////////////
		case SIM_FACTORY_TYPE_NONE:
        {
            printf("SIM Cannot spawn, entity type not set!\n");
            
            return NULL;
        } break;

        default:
        {
            ZE_ASSERT(0, "Sim Unknown entity type");
            return NULL;
        } break;
    }

    return ent;
}

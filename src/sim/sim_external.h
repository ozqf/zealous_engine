#pragma once
/*
sim.h implementation
*/
#include "sim.h"

extern "C" SimInventoryItem* SVI_GetItem(i32 index)
{
	if (index < 0) { index = 0; } 
	if (index >= SIM_INVENTORY_ITEM_COUNT) { index = SIM_INVENTORY_ITEM_COUNT - 1; }

	return &g_items[index];
}

extern "C" void Sim_PrepareSpawnData(
    SimScene* sim, SimEntSpawnData* data,
    i32 bIsLocal, u8 factoryType,
    Vec3 pos)
{
    *data = {};
    data->isLocal = bIsLocal;
    data->serial = Sim_ReserveEntitySerial(sim, bIsLocal);
    data->pos = pos;
    data->factoryType = factoryType;
}

extern "C"
i32 Sim_CalcEntityArrayBytes(i32 capacity)
{ return (sizeof(SimEntity) * capacity); }

extern "C"
i32 Sim_GetFrameNumber(SimScene* sim){ return sim->tick; }

extern "C"
void Sim_SimpleMove(SimEntity* ent, timeFloat delta)
{
    Vec3* pos = &ent->body.t.pos;
    ent->body.previousPos.x = pos->x;
    ent->body.previousPos.y = pos->y;
    ent->body.previousPos.z = pos->z;
    Vec3 move =
    {
        ent->movement.velocity.x * (f32)delta,
        ent->movement.velocity.y * (f32)delta,
        ent->movement.velocity.z * (f32)delta
    };
    
    ent->body.t.pos.x += move.x;
    ent->body.t.pos.y += move.y;
    ent->body.t.pos.z += move.z;
}

extern "C"
i32 Sim_InBounds(SimEntity* ent, Vec3* min, Vec3* max)
{
    Vec3* p = &ent->body.t.pos;
    if (p->x < min->x) { return NO; }
    if (p->x > max->x) { return NO; }
    if (p->y < min->y) { return NO; }
    if (p->y > max->y) { return NO; }
    if (p->z < min->z) { return NO; }
    if (p->z > max->z) { return NO; }
    return YES;
}

extern "C"
void Sim_BoundaryBounce(SimEntity* ent, Vec3* min, Vec3* max)
{
    Vec3* p = &ent->body.t.pos;
    if (p->x < min->x)
    { p->x = min->x; ent->movement.velocity.x = -ent->movement.velocity.x; }
    if (p->x > max->x)
    { p->x = max->x; ent->movement.velocity.x = -ent->movement.velocity.x; }

    if (p->y < min->y)
    { p->y = min->y; ent->movement.velocity.y = -ent->movement.velocity.y; }
    if (p->y > max->y)
    { p->y = max->y; ent->movement.velocity.y = -ent->movement.velocity.y; }

    if (p->z < min->z)
    { p->z = min->z; ent->movement.velocity.z = -ent->movement.velocity.z; }
    if (p->z > max->z)
    { p->z = max->z; ent->movement.velocity.z = -ent->movement.velocity.z; }
}

extern "C"
void Sim_BoundaryStop(SimEntity* ent, Vec3* min, Vec3* max)
{
    Vec3* p = &ent->body.t.pos;
    if (p->x < min->x)
    { p->x = min->x; }
    if (p->x > max->x)
    { p->x = max->x; }

    if (p->y < min->y)
    { p->y = min->y; }
    if (p->y > max->y)
    { p->y = max->y; }

    if (p->z < min->z)
    { p->z = min->z; }
    if (p->z > max->z)
    { p->z = max->z; }
}

////////////////////////////////////////////////////////////
// Targetting and validation
////////////////////////////////////////////////////////////

// If no then this ent must be ignored by all other entities
extern "C"
inline i32 Sim_IsEntInPlay(SimEntity* ent)
{
    if (ent == NULL) { return NO; }
    if (ent->id.serial == SIM_ENT_NULL_SERIAL) { return NO; }
    if (ent->status != SIM_ENT_STATUS_IN_USE) { return NO; }
    if ((ent->flags & SIM_ENT_FLAG_OUT_OF_PLAY)) { return NO; }
    return YES;
}

// If no then this ent cannot be attacked
extern "C"
inline i32 Sim_IsEntTargetable(SimEntity* ent)
{
    i32 result = Sim_IsEntInPlay(ent);
    if (result == NO) { return NO; }
    return ((ent->flags & SIM_ENT_FLAG_SHOOTABLE) != NO);
}

// If no then this ent cannot be attacked
/*extern "C"
inline i32 Sim_IsEntHurtable(SimEntity* ent)
{
    if (!(ent->flags & SIM_ENT_FLAG_SHOOTABLE)) { return NO; }
    return Sim_IsEntInPlay(ent);
}*/

/**
 * Returns NULL if no suitable target can be found
 */
extern "C"
SimEntity* Sim_FindTargetForEnt(SimScene* sim, SimEntity* subject)
{
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (Sim_IsEntInPlay(ent) == NO)
        { continue; }
        if (ent->factoryType != SIM_FACTORY_TYPE_ACTOR)
        { continue; }
        subject->relationships.targetId = ent->id;
        return ent;
    }
    subject->relationships.targetId = {};
    return NULL;
}

/*
NOTE: Was going to use to look for entity sets that are all dead
even though a player has an enqueued spawn message for them (packet loss etc)
However, probably wouldn't work. You can only guarentee that the player
HAS executed a command, not that they definitely haven't. It could be queued up
just not ack'd. yet. So they WOULD spawn stuff and this scan would be invalid
Search for number of entities in a range that are assigned
*/
#if 1
extern "C"
i32 Sim_ScanForSerialRange(SimScene* sim, i32 firstSerial, i32 numSerials)
{
    if (numSerials == 0) { return 0; }
    i32 count = 0;
    i32 lastSerial = firstSerial + (numSerials - 1);
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status == SIM_ENT_STATUS_FREE) { continue; }
        if (ent->id.serial >= firstSerial && ent->id.serial <= lastSerial)
        {
            count++;
        }
    }
    return count;
}
#endif
extern "C"
SimEntity* Sim_GetEntityBySerial(SimScene* sim, i32 serial)
{
    if (serial == SIM_ENT_NULL_SERIAL) { return NULL; }
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status == SIM_ENT_STATUS_FREE) { continue; }
        if (ent->id.serial == serial) { return ent; }
    }
    return NULL;
}

extern "C"
SimEntity* Sim_GetEntityByIndex(SimScene* sim, SimEntIndex index)
{
    SimEntity* ent = &sim->ents[index.index];
    if (ent->id.slot.iteration == index.iteration
        && ent->id.serial != 0)
    {
        return ent;
    }
    return NULL;
}

extern "C"
i32 Sim_ReserveEntitySerial(SimScene* scene, i32 isLocal)
{
    if (isLocal) { return scene->localEntitySequence--; }
    else
    {
        return scene->remoteEntitySequence++;
    }
}

extern "C"
i32 Sim_ReserveEntitySerials(
    SimScene* scene, i32 isLocal, i32 count)
{
    i32 first;
    i32 last;
    if (isLocal)
    {
        first = scene->localEntitySequence;
        last = first - count;
        scene->localEntitySequence = last;
        if (scene->bVerbose)
        {
            APP_LOG(128,
                "SIM Reserving %d local entity serials (%d to %d)\n",
                count, first, (last - 1)
            );
        }
    }
    else
    {
        first = scene->remoteEntitySequence;
        last = first + count;
        scene->remoteEntitySequence = last;
        if (scene->bVerbose)
        {
            APP_LOG(128,
                "SIM Reserving %d replicated entity serials (%d to %d)\n",
                count, first, (last - 1)
            );
        }
        
    }
    return first;
}

/**
 * Restore the exact state of a single entity
 * (create it if it doesn't exist)
 */
extern "C"
SimEntity* Sim_RestoreEntity(SimScene* scene, SimEntSpawnData* def)
{
	// an id of zero is considered invalid
	ZE_ASSERT(def->serial != SIM_ENT_NULL_SERIAL,
        "Restoring entity with null Serial");
    SimEntity* ent = Sim_GetEntityBySerial(scene, def->serial);
    // TODO: Handle this!
    ZE_ASSERT(ent == NULL, "Entity already exists!")
    ent = Sim_SpawnEntity(scene, def);
    return ent;
}

extern "C"
i32 Sim_RemoveEntity(SimScene* scene, i32 serialNumber)
{
	ZE_ASSERT(serialNumber != SIM_ENT_NULL_SERIAL,
        "Removing entity with null serial")
    return Sim_RecycleEntity(scene, serialNumber);
}

extern "C"
i32 Sim_SetActorInput(
    SimScene* sim,
    SimActorInput* input,
    i32 entitySerial)
{
    SimEntity* ent = Sim_FindEntityBySerialNumber(sim, entitySerial);
    if (!ent)
    {
        return ZE_ERROR_NOT_FOUND;
    }
    ent->input = *input;
    return ZE_ERROR_NONE;
}

extern "C"
i32 Sim_ExecuteBulkSpawn(
    SimScene* sim,
    SimBulkSpawnEvent* event,
	i32 fastForwardTicks,
    i32* spawnedEntityFlags,
    f32* spawnedEntityPriority)
{
    ZE_ASSERT(event->factoryType, "Bulk spawn factory type is 0")
    i32 isLocal = (event->base.firstSerial < 0);
    
    SimSpawnPatternItem items[255];
    ZE_ASSERT(event->patternDef.numItems < 256,
        "Pattern items > 255")
    i32 count = Sim_CreateSpawnPattern(
        &event->base,
        &event->patternDef,
        items,
        event->base.firstSerial,
        isLocal);
    ZE_ASSERT(count > 0, "Pattern count underflow")
    for (i32 i = 0; i < count; ++i)
    {
        SimSpawnPatternItem* item = &items[i];
		
        SimEntSpawnData entDef = {};
        entDef.factoryType = event->factoryType;
        entDef.serial = item->entSerial;
        entDef.pos = item->pos;
        entDef.scale = { 1, 1, 1 };
        entDef.birthTick = sim->tick - fastForwardTicks;
        entDef.fastForwardTicks = fastForwardTicks;
        entDef.parentSerial = event->base.sourceSerial;
        SimEntity* ent = Sim_RestoreEntity(sim, &entDef);
        
        ent->movement.velocity.x = (-item->forward.x) * ent->movement.speed;
        ent->movement.velocity.y = (-item->forward.y) * ent->movement.speed;
        ent->movement.velocity.z = (-item->forward.z) * ent->movement.speed;
        
        // TODO: Hack! Find Better way to return new entity info
        // The caller needs to know whether or not to track these
        // entities for priority queue sync.
        *spawnedEntityFlags = ent->flags;
        *spawnedEntityPriority = ent->basePriority;
    }

    return ZE_ERROR_NONE;
}

extern "C"
void Sim_Reset(SimScene* sim)
{
	i32 arraySize = Sim_CalcEntityArrayBytes(sim->maxEnts);
	i32 numBytes = sim->maxEnts * sizeof(SimEntity);
    ZE_SET_ZERO(sim->ents, arraySize)
	//sim->cmdSequence = 0;
	sim->tick = 0;
	// 0 == an invalid serial for error handling. Means once less
	// replicated entity, oh well
	sim->remoteEntitySequence = 1;
	sim->localEntitySequence = -1;
}

internal void Sim_PhysicsError(char* msg)
{
    ZE_ASSERT(0, msg)
}

extern "C"
void Sim_Init(
            char* label,
            SimScene* sim,
            SimEntity* entityMemory,
            i32 maxEntities)
{
    *sim = {};
    
    sim->ents = entityMemory;
    sim->maxEnts = maxEntities;
	sim->maxPlayers = SIM_MAX_PLAYERS;
    sim->bVerbose = NO;
    SVI_InitItemDefs();
	Sim_Reset(sim);
    #ifdef SIM_USE_PHYSICS_ENGINE
    sim->world = PhysExt_Create(label, Sim_PhysicsError);
    #endif
    //PhysExt_Init(NULL, 0, NULL, 0, NULL);
}

/*
Load world geometry and non-replicated entities.
ALL entities created here should be local!
*/
extern "C"
i32 Sim_LoadStaticScene(SimScene* sim, i32 index)
{
    return Sim_LoadEmbeddedScene(sim, index);
}

extern "C"
i32 Sim_Tick(
    SimScene* sim,
    ZEByteBuffer* input,
    ZEByteBuffer* output,
    timeFloat delta)
{
    Sim_ExecuteCommands(sim, input, delta);
    Sim_TickEntities(sim, output, delta);
    return ZE_ERROR_NONE;
}

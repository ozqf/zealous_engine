#pragma once
/*
sim.h implementation

TODO: Rename this file, it WAS exclusively external functions
but that is pointless. It is general misc crap now
*/
#include "sim.h"

extern "C" SimInventoryItem* SVI_GetItem(i32 index)
{
	if (index < 0) { index = 0; } 
	if (index >= SIM_INVENTORY_ITEM_COUNT) { index = SIM_INVENTORY_ITEM_COUNT - 1; }

	return &g_items[index];
}

extern "C" timeFloat Sim_GetFrameInterval(SimScene* sim)
{
    return (timeFloat)(1.0f / (f32)sim->info.tickRate);
}

/**
 * Calculate the tick number for a think event in the future.
 */
extern "C" frameInt Sim_CalcThinkTick(SimScene* sim, timeFloat secondsToThink)
{
    timeFloat result = secondsToThink / Sim_GetFrameInterval(sim);
    // round
    return sim->info.tick + (frameInt)(result + 0.5);
}

extern "C" void Sim_DumpCommandBuffer(SimScene* sim, ZEBuffer* buf)
{
    printf("=== Sim Scan commands (%d bytes) ===\n", buf->Written());
    ZCMD_BEGIN_ITERATE(buf)
        printf("Type %d (%d bytes)\n", cmdHeader->type, cmdHeader->size);
    ZCMD_END_ITERATE
}

extern "C" void Sim_PrepareSpawnData(
    SimScene* sim, SimEvent_Spawn* data,
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
i32 Sim_GetFrameNumber(SimScene* sim){ return sim->info.tick; }

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

/**
 * Returns NULL if no suitable target can be found
 */
extern "C"
SimEntity* Sim_FindTargetForEnt(SimScene* sim, SimEntity* subject)
{
    for (i32 i = 0; i < sim->info.maxEnts; ++i)
    {
        SimEntity* ent = &sim->data.ents[i];
        // only target players at the moment
        if (ent->factoryType != SIM_FACTORY_TYPE_ACTOR)
        { continue; }
        
        if (Sim_IsEntTargetable(ent) == NO)
        { continue; }
        if (SimEnt_CheckTeamDiffer(subject->teamId, ent->teamId) == NO)
        { continue; }

        // Target is good:
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
    for (i32 i = 0; i < sim->info.maxEnts; ++i)
    {
        SimEntity* ent = &sim->data.ents[i];
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
    for (i32 i = 0; i < sim->info.maxEnts; ++i)
    {
        SimEntity* ent = &sim->data.ents[i];
        if (ent->status == SIM_ENT_STATUS_FREE) { continue; }
        if (ent->id.serial == serial) { return ent; }
    }
    return NULL;
}

extern "C"
SimEntity* Sim_GetEntityByIndex(SimScene* sim, SimEntIndex index)
{
    SimEntity* ent = &sim->data.ents[index.index];
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
    if (isLocal) { return scene->info.localEntitySequence--; }
    else
    {
        return scene->info.remoteEntitySequence++;
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
        first = scene->info.localEntitySequence;
        last = first - count;
        scene->info.localEntitySequence = last;
        if (scene->info.bVerbose)
        {
            APP_LOG(128,
                "SIM Reserving %d local entity serials (%d to %d)\n",
                count, first, (last - 1)
            );
        }
    }
    else
    {
        first = scene->info.remoteEntitySequence;
        last = first + count;
        scene->info.remoteEntitySequence = last;
        if (scene->info.bVerbose)
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
SimEntity* Sim_RestoreEntity(SimScene* scene, SimEvent_Spawn* def)
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
    SimEvent_BulkSpawn* event,
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
		
        SimEvent_Spawn entDef = {};
        entDef.factoryType = event->factoryType;
        entDef.teamId = event->base.teamId;
        entDef.serial = item->entSerial;
        entDef.pos = item->pos;
        Vec3 euler = Vec3_EulerAngles(item->forward);
        entDef.pitchDegrees = euler.x * RAD2DEG;
        entDef.yawDegrees = euler.y * RAD2DEG;
        entDef.scale = { 1, 1, 1 };
        entDef.birthTick = sim->info.tick - fastForwardTicks;
        entDef.fastForwardTicks = fastForwardTicks;
        entDef.parentSerial = event->base.sourceSerial;
        SimEntity* ent = Sim_RestoreEntity(sim, &entDef);
        
        // set velocity based on entity speed:
        ent->movement.velocity.x = (-item->forward.x) * ent->movement.speed;
        ent->movement.velocity.y = (-item->forward.y) * ent->movement.speed;
        ent->movement.velocity.z = (-item->forward.z) * ent->movement.speed;
        // set desired move dir if applicable
        if (ent->movement.moveMode == SIM_ENT_MOVE_TYPE_WALK
            || ent->movement.moveMode == SIM_ENT_MOVE_TYPE_FLOAT)
        {
            ent->movement.move = Vec3_Normalised(ent->movement.velocity);
        }

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
	i32 arraySize = Sim_CalcEntityArrayBytes(sim->info.maxEnts);
	i32 numBytes = sim->info.maxEnts * sizeof(SimEntity);
    ZE_SET_ZERO(sim->data.ents, arraySize)
	//sim->cmdSequence = 0;
	sim->info.tick = 0;
	// 0 == an invalid serial for error handling.
	sim->info.remoteEntitySequence = 1;
	sim->info.localEntitySequence = -1;
    sim->info.numActivePlayers = 0;
    sim->info.nextPlayerId = 1;
    ZE_SET_ZERO(sim->data.players, sizeof(SimPlayer) * sim->info.maxPlayers)
}

internal void Sim_PhysicsError(char* msg)
{
    ZE_ASSERT(0, msg)
}

extern "C"
void Sim_Init(
            ZE_FatalErrorFunction fatalFunc,
            char* label,
            SimScene* sim,
            SimEntity* entityMemory,
            i32 maxEntities)
{
    ZE_SetFatalError(fatalFunc);

    *sim = {};
    
    sim->data.ents = entityMemory;
    sim->info.maxEnts = maxEntities;
	sim->info.maxPlayers = SIM_MAX_PLAYERS;
    sim->info.bVerbose = NO;
    sim->info.tickRate = 60;
    sim->info.groundOrigin = { };
    sim->info.groundNormal = { 0, 1, 0 };
    sim->info.gravity = { 0, -12.f, 0 };
    SVI_InitItemDefs();
	Sim_Reset(sim);
    Sim_InitTickFunctions(sim);
    #ifdef SIM_USE_PHYSICS_ENGINE
    sim->world = PhysExt_Create(label, Sim_PhysicsError);
    #endif
	
    //PhysExt_Init(NULL, 0, NULL, 0, NULL);
}

/*
Load entities from a map file.
if bLocalOnly, do not load any dynamic (replicated) entities,
just static/geometry
*/
extern "C"
i32 Sim_LoadMapFile(SimScene* sim, const char* mapName, i32 bLocalOnly)
{
	i32 index = ZE_AsciToInt32(mapName);
    
    return Sim_LoadEmbeddedScene(sim, index, bLocalOnly);
}

extern "C"
i32 Sim_Tick(
    SimScene* sim,
    ZEBuffer* input,
    ZEBuffer* output,
    ZEBuffer* soundOutput,
    timeFloat delta)
{
    sim->data.outputBuf = output;
    sim->data.soundOutputBuf = soundOutput;
    Sim_ExecuteCommands(sim, input, delta);
    SimPlyr_Tick(sim);
    Sim_TickEntities(sim, output, delta);
    SimRules_PostUpdate(sim, delta);
    return ZE_ERROR_NONE;
}

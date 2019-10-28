#pragma once

#include "../../ze_common/ze_common_full.h"
#include "server.cpp"


internal i32 SVU_CreateSalt()
{
    i32 result = 0;
    do
    {
        result = (i32)(COM_STDRandf32() * INT_MAX);
    } while (result == 0);
    return result;
}

internal i32 SVU_IsPrivateIdInUse(i32 id)
{
    for (i32 i = 0; i < g_users.max; ++i)
    {
        User* u = &g_users.items[i];
        if (u->ids.privateId == id)
        {
            return 1;
        }
    }
    return 0;
}

internal UserIds SVU_GenerateUserId()
{
    UserIds newId;
    do
    {
        newId.privateId = SVU_CreateSalt();
    } while (SVU_IsPrivateIdInUse(newId.privateId));
    newId.publicId = g_users.nextPublicId++;
    return newId;
}

internal void SVU_AllocateUserStream(
    NetStream* stream, i32 capacityPerBuffer)
{
    if (stream->initialised) { return; }
    stream->initialised = 1;
    stream->inputBuffer = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, capacityPerBuffer, "User Input"),
        capacityPerBuffer
    );
    stream->outputBuffer = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, capacityPerBuffer, "User Output"),
        capacityPerBuffer
    );
}

internal void SVU_EnqueueCommandForAllUsers(
    UserList* users, Command* cmd)
{
	for (i32 i = 0; i < users->max; ++i)
	{
		User* user = &users->items[i];
		if (user->state == USER_STATE_FREE)
		{ continue; }
	
		//
		Stream_EnqueueOutput(&user->reliableStream, cmd);
	}
}

internal void SVU_AddEntityLinkForAllUsers(
    UserList* users, i32 entSerial, f32 priority)
{
    for (i32 i = 0; i < users->max; ++i)
    {
        User* user = &users->items[i];
        if (user->state == USER_STATE_FREE) { continue; }
        Priority_AddLink(&user->entSync, entSerial, priority);
    }
}

internal void SVU_AddBulkEntityLinksForAllUsers( 
    UserList* users, i32 firstSerial, i32 numEntities, f32 priority)
{
    for (i32 i = 0; i < numEntities; ++i)
    {
        SVU_AddEntityLinkForAllUsers(users, firstSerial++, priority);
    }
}

internal void SVU_RemoveEntityForAllUsers(
    UserList* users, i32 entSerial)
{
    for (i32 i = 0; i < g_users.max; ++i)
    {
        User* u = &g_users.items[i];
        if (u->state == USER_STATE_FREE) { continue; }
        //SV_RemovePriorityLinkBySerial(&u->entSync, entSerial);
        Priority_FlagLinkAsDead(&u->entSync, entSerial);
    }
}

internal void SVU_StartUserSync(User* user)
{
    APP_PRINT(128, "SV - Begin sync for user %d\n",
        user->ids.privateId);
    NetStream* stream = &user->reliableStream;

    S2C_Sync sync;
    Cmd_InitSync(&sync, g_sim.tick, 8, user->entSerial);
    Stream_EnqueueOutput(stream, &sync.header);
    // start user command queue
    for (i32 j = 0; j < g_sim.maxEnts; ++j)
    {
        SimEntity* ent = &g_sim.ents[j];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
        if (ent->isLocal) { continue; }
        S2C_RestoreEntity cmd = {};

        // Add an entity link if required
        if (ent->flags & SIM_ENT_FLAG_POSITION_SYNC)
        {
            Priority_AddLink(&user->entSync, ent->id.serial, 1);
        }

        Cmd_InitRestoreEntity(&cmd, g_sim.tick, ent);
        
        ZEByteBuffer* b = &user->reliableStream.outputBuffer;
        Stream_EnqueueOutput(&user->reliableStream, (Command*)&cmd);
        APP_LOG(64, "  Write Entity %d\n", ent->id.serial);
    }
    APP_LOG(64, "SV User %d has %d sync bytes\n",
        user->ids.privateId, stream->outputBuffer.Written());
}

internal User* SVU_CreateUser(UserIds ids, ZNetAddress* addr)
{
    User* user = User_GetFree(&g_users);
    user->ids = ids;
    user->address = *addr;
    SVU_AllocateUserStream(&user->reliableStream, KiloBytes(64));
    SVU_AllocateUserStream(&user->unreliableStream, KiloBytes(64));

    user->entSync = {};
    user->entSync.links = (PriorityLink*)COM_Malloc(
        &g_mallocs,
        Priority_CalcEntityLinkArrayBytes(APP_MAX_ENTITIES),
        "EntLinks");
    user->entSync.maxLinks = APP_MAX_ENTITIES;
    user->entSync.numLinks = 0;
    //user->syncRateHertz = APP_CLIENT_SYNC_RATE_10HZ;
    //user->syncRateHertz = APP_CLIENT_SYNC_RATE_20HZ;
    //user->syncRateHertz = APP_CLIENT_SYNC_RATE_30HZ;
    user->syncRateHertz = APP_CLIENT_SYNC_RATE_60HZ;

    APP_LOG(64, "SV creating new user public %d private %d\n",
        ids.publicId, ids.privateId
    );
    return user;
}

internal void SVU_SpawnUserAvatar(User* u)
{
	SimEntSpawnData def = {};
    def = {};
    //def.isLocal = 1;
	i32 avatarSerial = Sim_ReserveEntitySerial(&g_sim, 0);
	u->entSerial = avatarSerial;
    def.serial = avatarSerial;
	def.factoryType = SIM_FACTORY_TYPE_ACTOR;
    def.pos = { -6, 0, 6 };
    def.scale = { 1, 1, 1 };
    Sim_RestoreEntity(&g_sim, &def);
}

UserIds SVU_CreateLocalUser()
{
    ZNetAddress addr = {};
    addr.port = APP_CLIENT_LOOPBACK_PORT;
    UserIds id = SVU_GenerateUserId();
    User* u = SVU_CreateUser(id, &addr);
    u->state = USER_STATE_SYNC;
	SVU_SpawnUserAvatar(u);
    SVU_StartUserSync(u);
    return id;
}

internal void SVU_ClearStaleOutput(User* user, SimScene* sim, ZEByteBuffer* output)
{
    u8* read = output->start;
    u8* end = read + output->Written();
    while(read < end)
    {
        Command* cmd = (Command*)read;
		ErrorCode err = Cmd_Validate(cmd);
		ZE_ASSERT(err == ZE_ERROR_NONE, "Bad command")
        read += cmd->size;
        if (cmd->type != CMD_TYPE_S2C_BULK_SPAWN) { continue; }

        S2C_BulkSpawn* spawn = (S2C_BulkSpawn*)cmd;
        i32 firstSerial = spawn->def.base.firstSerial;
        i32 numSerials = spawn->def.patternDef.numItems;
        i32 numAlive = Sim_ScanForSerialRange(&g_sim,
            firstSerial,
            numSerials);
        if (numAlive > 0)
        {
            // This event is still relevant because entities from it
            // still exist
            continue;
        }
        // Replace with a skip command.
        // printf("SVP User has outdated spawn event! (Id %d to %d)\n",
        //     firstSerial, numSerials);
        #if 1
        S2C_RemoveEntityGroup grp = {};
        Cmd_InitRemoveEntityGroup(&grp, cmd->tick, firstSerial, (u8)numSerials);
        // Remember to copy the reliable sequence number and tick!
        grp.header.sequence = cmd->sequence;

        i32 spaceRequired = sizeof(S2C_RemoveEntityGroup);
        Stream_DeleteCommand(output, cmd, spaceRequired);
        COM_COPY(&grp, cmd, spaceRequired);
		read = (u8*)cmd + spaceRequired;
        end = output->start + output->Written();
        
        // No need for any links either
        Priority_DeleteLinkRange(&user->entSync, firstSerial, numSerials);
        #endif
    }
}

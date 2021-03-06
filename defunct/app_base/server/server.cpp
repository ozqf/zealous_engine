#ifndef SERVER_CPP
#define SERVER_CPP

#include <stdlib.h>
#include "../../ze_common/ze_common_full.h"
#include "../app.h"
#include "../shared/commands.h"
#include "../shared/commands_serialise.h"
#include "../shared/commands_deserialise.h"
#include "../shared/stream.h"
#include "../shared/packet.h"
#include "../shared/packet.h"
#include "../../sys_events.h"
#include "../../sim/sim.h"
#include "server.h"
#include "server_priority.h"

#include "server_internal.h"

extern "C" i32 SV_IsRunning() { return g_isRunning; }

extern "C" void SV_Debug_GetSimInstance(void** ptr)
{
    *ptr = &g_sim;
}

#include "server_users.h"
#include "server_game.h"
#include "server_packets.h"
#include "server_debug.h"
#include "server_connect.h"

extern "C" u8 SV_ParseCommandString(const char* str, const char** tokens, const i32 numTokens)
{
	if (!ZE_CompareStrings(tokens[0], "SPAWN"))
	{
		if (numTokens == 1)
		{
			APP_PRINT(64, "SV === Spawn entity options: ===\n");
			APP_PRINT(64, "\tWANDERER\n");
			return 1;
		}
		if (numTokens == 2)
		{
			if (ZE_CompareStrings(tokens[1], "WANDERER"))
			{
				APP_PRINT(128, "\tSV Spawn Wanderer\n");
			}
			else
			{
				APP_PRINT(256, "\tUnknown entity type %s\n",
                    tokens[1]);
			}
			return 1;
		}
		APP_PRINT(64, "SV incorrect token count for spawn\n");
		return 1;
	}
    else if (numTokens == 2 && !ZE_CompareStrings(tokens[0], "RATE"))
    {
        i32 value = ZE_AsciToInt32(tokens[1]);
        printf("SV Set Sync rate %d\n", value);
        switch (value)
        {
            case 10:
            g_maxSyncRate = APP_CLIENT_SYNC_RATE_10HZ;
            return 1;
            case 20:
            g_maxSyncRate = APP_CLIENT_SYNC_RATE_20HZ;
            return YES;
            case 30:
            g_maxSyncRate = APP_CLIENT_SYNC_RATE_30HZ;
            return YES;
            case 60:
            g_maxSyncRate = APP_CLIENT_SYNC_RATE_60HZ;
            return YES;

            default:
            printf("Invalid sync rate %d\n", value);
            break;
        }
        return YES;
    }
    return NO;
}

internal void SV_AddWanderer()
{
    SimEvent_Spawn def = {};
    def = {};
    def.isLocal = 0;
    def.serial = Sim_ReserveEntitySerial(&g_sim, def.isLocal);
    printf("SV Reserver serial %d for Wanderer\n", def.serial);
    def.pos.x = COM_STDRandomInRange(-8, 8);
    def.pos.y = 0;
    def.pos.z = COM_STDRandomInRange(-8, 8);
    def.factoryType = SIM_FACTORY_TYPE_WANDERER;
    def.scale = { 1, 1, 1 };
    Sim_RestoreEntity(&g_sim, &def);
}

internal void SV_AddSpawner(
    SimScene* sim, Vec3 pos, simFactoryType factoryType, u8 spawnCount)
{
    SimEvent_Spawn data = {};
    data.numChildren = spawnCount;
    Sim_PrepareSpawnData(sim, &data, 1, SIM_FACTORY_TYPE_SPAWNER, pos);
    data.childFactoryType = factoryType;
    data.numChildren = spawnCount;
    data.patternType = SIM_PATTERN_FLAT_SCATTER;
    //data.patternType = SIM_PATTERN_3D_SCATTER;
    Sim_RestoreEntity(&g_sim, &data);
}

internal void SV_AddBot(SimScene* sim, Vec3 pos)
{
    SimEvent_Spawn data = {};
    Sim_PrepareSpawnData(sim, &data, 0, SIM_FACTORY_TYPE_BOT, pos);
    SimEntity* ent = Sim_RestoreEntity(&g_sim, &data);
    S2C_RestoreEntity cmd = {};
    Cmd_InitRestoreEntity(&cmd, sim->tick, ent);
    SVU_EnqueueCommandForAllUsers(&g_users, &cmd.header);
}

internal void SV_AddGrunt(SimScene* sim, Vec3 pos)
{
    SimEvent_Spawn data = {};
    Sim_PrepareSpawnData(sim, &data, 1, SIM_FACTORY_TYPE_GRUNT, pos);
    SimEntity* ent = Sim_RestoreEntity(&g_sim, &data);
    S2C_RestoreEntity cmd = {};
    Cmd_InitRestoreEntity(&cmd, sim->tick, ent);
}

internal void SV_LoadTestScene(i32 staticScene, i32 dynamicScene)
{
    SimScene* sim = &g_sim;
    Sim_LoadStaticScene(sim, staticScene);
    const i32 stage = dynamicScene;
    u8 count = 4;//64;
    f32 inner = 8;
    f32 outer = 12;

    APP_PRINT(128, "SV - Init scene %d\n", stage)

	switch (stage)
    {
        // No spawners
        case -1:
        break;
        case 1:
        SV_AddSpawner(sim, { outer, 0, outer }, SIM_FACTORY_TYPE_RUBBLE, count);
        //SV_AddSpawner(sim, { 0, 0, 0 }, SIM_FACTORY_TYPE_SEEKER, count);
        //SV_AddSpawner(sim, { 0, 10, 0 }, SIM_FACTORY_TYPE_SEEKER_FLYING, count);
        break;
        case 2:
        SV_AddSpawner(sim, { 10, 0, 10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { -10, 0, 10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { 10, 0, -10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { -10, 0, -10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { 0, 0, 0 }, SIM_FACTORY_TYPE_SEEKER, count);
        break;
        case 3:
        SV_AddSpawner(sim, { 10, 0, 10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { -10, 0, 10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { 10, 0, -10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { -10, 0, -10 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddSpawner(sim, { 0, 0, 0 }, SIM_FACTORY_TYPE_DART, count);
        break;
        case 4:
        SV_AddSpawner(sim, { -10, 0, 10 }, SIM_FACTORY_TYPE_BOUNCER, count);
        SV_AddSpawner(sim, { 10, 0, -10 }, SIM_FACTORY_TYPE_BOUNCER, count);
        SV_AddSpawner(sim, { 10, 0, 10 }, SIM_FACTORY_TYPE_DART, count);
        SV_AddSpawner(sim, { -10, 0, -10 }, SIM_FACTORY_TYPE_DART, count);
        SV_AddSpawner(sim, { 0, 0, 0 }, SIM_FACTORY_TYPE_SEEKER, count);
        SV_AddBot(sim, { 15, 0, 15 });
        SV_AddBot(sim, { 15, 0, -15 });
        SV_AddBot(sim, { 15, 0, 0 });
        break;
        case 5:
        SV_AddSpawner(sim, { inner, 0, inner }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { outer, 0, outer }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { -inner, 0, inner }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { -outer, 0, outer }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { inner, 0, -inner }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { outer, 0, -outer }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { -inner, 0, -inner }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { -outer, 0, -outer }, SIM_FACTORY_TYPE_RUBBLE, count);
        SV_AddSpawner(sim, { 0, 0, 0 }, SIM_FACTORY_TYPE_RUBBLE, count);
        // SV_AddBot(sim, { 15, 0, 15 });
        // SV_AddBot(sim, { 15, 0, -15 });
        // SV_AddBot(sim, { 15, 0, 0 });
        break;
        case 6:
        SV_AddSpawner(sim, { 10, 15, 10 }, SIM_FACTORY_TYPE_SEEKER_FLYING, count);
        SV_AddSpawner(sim, { -10, 15, 10 }, SIM_FACTORY_TYPE_SEEKER_FLYING, count);
        SV_AddSpawner(sim, { 10, 15, -10 }, SIM_FACTORY_TYPE_SEEKER_FLYING, count);
        SV_AddSpawner(sim, { -10, 15, -10 }, SIM_FACTORY_TYPE_SEEKER_FLYING, count);
        break;
        default:
        SV_AddSpawner(sim, { 0, 0, 0 }, SIM_FACTORY_TYPE_DART, 1);
        break;
    }
    i32 numWanderers = 0;
    for (i32 i = 0; i < numWanderers; ++i)
    {
        SV_AddWanderer();
    }
    if (numWanderers > 0)
    {
        printf("SV spawned %d wanderers\n", numWanderers);
    }
    
    APP_PRINT(128, "\tDone\n")
}

internal void SV_ListAllocs()
{
    printf("-- SV ALLOCS --\n");
    i32 tally = 0;
    for (i32 i = 0; i < g_mallocs.max; ++i)
    {
        MallocItem* item = &g_mallocs.items[i];
        tally += item->capacity;
        if (item->ptr == NULL) { continue; }
        printf("%s: %d bytes\n", item->label, item->capacity);
    }
    printf("  Total: %lluKB, %lluMB\n",
        BytesAsKB(g_mallocs.totalBytes),
        BytesAsMB(g_mallocs.totalBytes)
        );
    printf("    Tally: %d bytes\n", tally);
}

internal void SV_ResetEntityPositionRecords()
{
    i32 bytesPerFrame = sizeof(SVEntityFrame) * APP_MAX_ENTITIES;
    i32 bytesTotal = bytesPerFrame * SV_NUM_POSITION_FRAMES_RECORDED;
    if (g_entityRecords == NULL)
    {
        g_entityRecords = (SVEntityFrame*)COM_Malloc(
            &g_mallocs, bytesTotal, 0, "Entity Frames");
    }
    ZE_SET_ZERO((u8*)g_entityRecords, bytesTotal);
}

extern "C" void SV_Init()
{
    APP_PRINT(32, "SV - init\n");
}

extern "C" void SV_Start()
{
    g_isRunning = YES;
    APP_PRINT(64, "SV Init scene\n");

    SV_PrintMsgSizes();

    g_mallocs = COM_InitMallocList(g_mallocItems, SV_MAX_MALLOCS);

    g_users = {};
    g_users.max = APP_MAX_USERS;
    i32 userBytes = sizeof(User) * APP_MAX_USERS;
    User* users = (User*)COM_Malloc(&g_mallocs, userBytes, 0, "SV Users");
    g_users.items = users;

    SV_ResetEntityPositionRecords();

    i32 size;// = KiloBytes(64);

    i32 maxEnts = APP_MAX_ENTITIES;
    size = Sim_CalcEntityArrayBytes(maxEnts);
    SimEntity* mem = (SimEntity*)COM_Malloc(&g_mallocs, size, 0, "Sim Ents");
    Sim_Init("Server", &g_sim, mem, maxEnts);
	Sim_Reset(&g_sim);
    // Inflate sim frame number - debugging
    g_sim.tick += 123;
    SV_LoadTestScene(g_staticSceneIndex, SV_DEFAULT_DYNAMIC_SCENE);

    SV_ListAllocs();
}

extern "C" void SV_Shutdown()
{
    g_isRunning = NO;
    for (i32 i = 0; i < g_mallocs.max; ++i)
    {
        if (g_mallocs.items[i].ptr != NULL)
        {
            free(g_mallocs.items[i].ptr);
        }
    }
}

internal void SV_ReadSystemEvents(ZEByteBuffer* sysEvents, timeFloat deltaTime)
{
    AppTimer timer(APP_STAT_SV_INPUT, g_sim.tick);

	u8* read = sysEvents->start;
	u8* end = sysEvents->cursor;
	while (read < end)
	{
		SysEvent* ev = (SysEvent*)read;
		i32 err = Sys_ValidateEvent(ev);
		if (err != ZE_ERROR_NONE)
		{
			printf("SV Error %d reading system event header\n", err);
			return;
		}
		read += ev->size;
		switch (ev->type)
		{
			case SYS_EVENT_PACKET:
            {
				//COM_PrintBytes((u8*)ev, ev->size, 16);
                SysPacketEvent* packet = (SysPacketEvent*)ev;
				// Don't read packets intended for the client
				if (packet->sender.port == APP_CLIENT_LOOPBACK_PORT)
				{
					continue;
				}
                SVP_ReadPacket(packet, &g_sim.quantise, g_elapsed);
            } break;

            case SYS_EVENT_INPUT:
            {
                printf("SV Input - skip\n");
            } break;

            case SYS_EVENT_SKIP: break;
		}
	}
}

internal void SV_SendUserPackets(SimScene* sim, timeFloat deltaTime)
{
    AppTimer timer(APP_STAT_SV_OUTPUT, g_sim.tick);
    for (i32 i = 0; i < g_users.max; ++i)
	{
		User* user = &g_users.items[i];
		if (user->state == USER_STATE_FREE) { continue; }

        // Clean out reliable queue if possible
        SVU_ClearStaleOutput(user, sim, &user->reliableStream.outputBuffer);

        // Update priorities
        SimEntity* avatar = Sim_GetEntityBySerial(sim, user->entSerial);
        if (avatar != NULL)
        {
            SVP_CalculatePriorities(
                sim, avatar, user->entSync.links, user->entSync.numLinks);
        }
        user->packetStats[sim->tick % USER_NUM_PACKET_STATS] = {};
        // force sending rate
        i32 rate;
        if (user->syncRateHertz < g_maxSyncRate)
        {
            rate = user->syncRateHertz;
        }
        else
        {
            rate = g_maxSyncRate;
        }
        
        switch (rate)
        {
            case APP_CLIENT_SYNC_RATE_30HZ:
            if (sim->tick % 2 != 0) { continue; }
            break;
            case APP_CLIENT_SYNC_RATE_20HZ:
            if (sim->tick % 3 != 0) { continue; }
            break;
            case APP_CLIENT_SYNC_RATE_10HZ:
            if (sim->tick % 6 != 0) { continue; }
            break;
            case APP_CLIENT_SYNC_RATE_60HZ:
            default:
            break;
        }

		Priority_TickQueue(&user->entSync);
        
        PacketStats stats = SVP_WriteUserPacket(sim, user, g_elapsed);
        user->packetStats[sim->tick % USER_NUM_PACKET_STATS] = stats;

        // add to totals
        user->streamStats.numPackets += 1;
        user->streamStats.totalBytes += stats.totalBytes;
        user->streamStats.reliableBytes += stats.reliableBytes;
        user->streamStats.unreliableBytes += stats.unreliableBytes;
        user->streamStats.numReliableMessages += stats.numReliableMessages;
        user->streamStats.numReliableSkipped += stats.numReliableSkipped;
        user->streamStats.numUnreliableMessages += stats.numUnreliableMessages;

        //printf(".");
        // Add up command stats here
        for (i32 j = 0; j < 256; ++j)
        {
            user->streamStats.commandCounts[j] += stats.commandCounts[j];
        }
	}
}

internal void SV_CalcPings(timeFloat deltaTime)
{
    for (i32 i = 0; i < g_users.max; ++i)
    {
        User* u = &g_users.items[i];
        if (u->state == USER_STATE_FREE) { continue; }
        AckStream* acks = &u->acks;
        u->ping = Ack_CalculateAverageDelay(acks);
        u->jitter = (acks->delayMax - acks->delayMin);
    }
}

extern "C" void SV_Tick(ZEByteBuffer* sysEvents, timeFloat deltaTime)
{
    APP_LOG(64, "*** SV TICK %d (T %.3f) ***\n", g_sim.tick, g_elapsed);
    SV_ReadSystemEvents(sysEvents, deltaTime);

    ZNetAddress addr = {};
    addr.port = APP_CLIENT_LOOPBACK_PORT;
    User* user = User_FindByAddress(&g_users, &addr);
    if (user != NULL)
    {
        APP_LOG(256, "SV Local player ping %.5f, jitter %.5f, Sim tick %d, Input Seq %d\n",
            user->ping, user->jitter, user->latestServerTick, user->userInputSequence
        );
    }
    
    SV_CalcPings(deltaTime);
    SVG_TickSim(&g_sim, deltaTime);
    
	g_elapsed += deltaTime;
    SV_SendUserPackets(&g_sim, deltaTime);
}
/*
void SV_PopulateRenderScene(
    RenderScene* scene,
    i32 maxObjects,
    i32 texIndex,
    f32 interpolateTime,
    i32 g_debugDrawServerScene,
    i32 g_debugDrawServerTests)
{
    if (!g_debugDrawServerScene && !g_debugDrawServerTests) { return; }
    for (i32 j = 0; j < g_sim.maxEnts; ++j)
    {
        SimEntity* ent = &g_sim.ents[j];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }

        RendObj obj = {};
        //MeshData* cube = COM_GetCubeMesh();
        //RendObj_SetAsMesh(
        //    &obj, *cube, 1, 0, 0, texIndex);

        switch(ent->tickType)
        {
            case SIM_TICK_TYPE_LINE_TRACE:
            {
                if (!g_debugDrawServerTests) { break; }
                Vec3* a = &ent->body.t.pos;
                Vec3* b = &ent->destination;
                // move ray up slightly out of the floor
                f32 offsetY = 0.2f;
                RendObj_SetAsLine(&obj,
                    { a->x, a->y + offsetY, a->z },
                    { b->x, b->y + offsetY, b->z },
                    COL_U32_PURPLE,
                    COL_U32_PURPLE
                );
                TRANSFORM_CREATE(t);
                RScene_AddRenderItem(scene, &t, &obj);
            } break;

            default:
            {
                if (!g_debugDrawServerScene) { break; }
                RendObj_SetAsAABB(
			        &obj, 1, 1, 1, 0, 1, 0);
                RScene_AddRenderItem(scene, &ent->body.t, &obj);
            } break;
        }
    }
}
*/
#endif // SERVER_CPP

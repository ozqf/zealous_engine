#include "game_server.h"
#include "../../sim/sim.h"

internal i32 g_bIsRunning = NO;
internal SimScene* g_sim = NULL;

extern "C" void SV_Init(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* db)
{
	
}

extern "C" void SV_Start(SimScene* sim)
{
	printf("SV Start\n");
	g_bIsRunning = YES;
    g_sim = sim;
}

/**
 * Returns player Id
 */
extern "C" i32 SV_CreateLocalPlayer(SimScene* sim, ZEBuffer* buf)
{
    i32 playerId = SimPlyr_ReserveId(sim);
    //i32 avatarId = Sim_ReserveEntitySerial(sim, NO);

    ZE_INIT_PTR_IN_PLACE(plyr, SimEvent_PlayerState, buf);
    plyr->Set(
        playerId, SIM_ENT_NULL_SERIAL, SIM_PLAYER_STATE_OBSERVING, sim->info.tick);
    return playerId;

    #if 0
    SimPlayer* plyr = SimPlyr_Create(sim, 0);
    // reserve the Id for this player's avatar although we're not
    // necessarily spawning it yet.
    plyr->avatarId = Sim_ReserveEntitySerial(sim, NO);
    return *plyr;
    #endif
}

extern "C" void SV_KillPlayer()
{
    printf("SV - kill players\n");
    i32 len = g_sim->info.maxPlayers;
    for (i32 i = 0; i < len; ++i)
    {
        if (g_sim->data.players[i].state != SIM_PLAYER_STATE_IN_GAME)
        { continue; }
        if (g_sim->data.players[i].avatarId == 0)
        { continue; }
    }
}

extern "C" void GSV_Stop()
{
    printf("SV Stop\n");
	g_bIsRunning = NO;
    g_sim = NULL;
}

extern "C" void SV_Save(
    SimScene* sim, SimSaveFileInfo* saveInfo, i32 file, ZEFileIO files)
{
    saveInfo->server = files.FilePosition(file);
    saveInfo->numServerBytes = sizeof(i32);
    files.WriteToFile(file, (u8*)&g_bIsRunning, sizeof(i32));
}

extern "C" void SV_Resume(
    SimScene* sim, SimSaveFileInfo* saveInfo, ZEBuffer* saveData)
{
    if (saveInfo->numServerBytes != sizeof(i32))
    {
        printf("SV read incorrect size in save file\n");
        return;
    }
    u8* read = saveData->GetAtOffset(saveInfo->server);
    g_bIsRunning = *((i32*)read);
}

extern "C" void SV_PreTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

extern "C" void SV_PostTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta)
{
	if (!g_bIsRunning) { return; }
    #if 0
    u8* read = buf->GetWrite()->start;
    u8* end = buf->GetWrite()->cursor;
    while (read < end)
    {
        ZECommand* cmd = (ZECommand*)read;
        read += cmd->size;
        printf("SV read output cmd type %d (%d bytes\n", cmd->type, cmd->size);
    }
    #endif
}

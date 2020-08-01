#include "game_server.h"
#include "../../sim/sim.h"

internal i32 g_bIsRunning = NO;

extern "C" void GSV_Init()
{
	
}

extern "C" void GSV_Start(SimScene* sim)
{
	printf("SV Start\n");
	g_bIsRunning = YES;
}

extern "C" SimPlayer GSV_CreateLocalPlayer(SimScene* sim, ZEByteBuffer* buf)
{
    SimPlayer* plyr = SimPlyr_Create(sim);
    // reserve the Id for this player's avatar although we're not
    // necessarily spawning it yet.
    plyr->avatarId = Sim_ReserveEntitySerial(sim, NO);
    #if 0 // spawn avatar
    SimEvent_Spawn* cmd = (SimEvent_Spawn*)buf->cursor;
    *cmd = {};
    cmd->header.type = SIM_CMD_TYPE_RESTORE_ENTITY;
    cmd->header.sentinel = ZCMD_SENTINEL;
    cmd->header.size = sizeof(SimEvent_Spawn);
    cmd->factoryType = SIM_TICK_TYPE_ACTOR;
    cmd->pos = sim->playerStartPos;
    cmd->serial = plyr->avatarId;

    buf->cursor += cmd->header.size;
    #endif
	return *plyr;
}

extern "C" void GSV_Stop()
{
    printf("SV Stop\n");
	g_bIsRunning = NO;
}

extern "C" void SV_PreTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

extern "C" void SV_PostTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta)
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

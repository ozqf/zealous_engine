#include "game_server.h"
#include "../../sim/sim.h"

internal i32 g_bIsRunning = NO;

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
    Sim_RestoreEntity(sim, &data);
}

extern "C" void GSV_Init()
{
	
}

extern "C" void GSV_Start(SimScene* sim)
{
	printf("SV Start\n");
	g_bIsRunning = YES;
	SV_AddSpawner(sim, {}, SIM_FACTORY_TYPE_WANDERER, 10);
}

extern "C" SimPlayer GSV_CreateLocalPlayer(SimScene* sim)
{
	SimPlayer* plyr = SimPlyr_Create(sim);
	plyr->avatarId = Sim_ReserveEntitySerial(sim, NO);
	return *plyr;
}

extern "C" void GSV_Stop()
{
    printf("SV Stop\n");
	g_bIsRunning = NO;
}

extern "C" void GSV_PreTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

extern "C" void GSV_PostTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta)
{
	if (!g_bIsRunning) { return; }
    u8* read = buf->GetWrite()->start;
    u8* end = buf->GetWrite()->cursor;
    while (read < end)
    {
        ZECommand* cmd = (ZECommand*)read;
        read += cmd->size;
        printf("SV read output cmd type %d (%d bytes\n", cmd->type, cmd->size);
    }
}

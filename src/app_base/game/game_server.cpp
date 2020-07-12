#include "game_server.h"
#include "../../sim/sim.h"

internal i32 g_bIsRunning = NO;

extern "C" void GSV_Init()
{
	
}

extern "C" void GSV_Start()
{
	printf("SV Start\n");
	g_bIsRunning = YES;
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

extern "C" void GSV_PreTick(timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

extern "C" void GSV_PostTick(timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

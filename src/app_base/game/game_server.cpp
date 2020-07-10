#include "game_server.h"

internal i32 g_bIsRunning = NO;

extern "C" void SV_Init()
{
	
}

extern "C" void SV_Start()
{
	printf("SV Start\n");
	g_bIsRunning = YES;
}

extern "C" void SV_Stop()
{
    printf("SV Stop\n");
	g_bIsRunning = NO;
}

extern "C" void SV_PreTick(timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

extern "C" void SV_PostTick(timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

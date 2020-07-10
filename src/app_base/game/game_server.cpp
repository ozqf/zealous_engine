#include "game_server.h"

internal i32 g_bIsRunning = NO;

extern "C" void SV_Init()
{
	
}

extern "C" void SV_PreTick(timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

extern "C" void SV_PostTick(timeFloat delta)
{
	if (!g_bIsRunning) { return; }
}

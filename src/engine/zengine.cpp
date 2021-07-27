#include "../../headers/zengine.h"
#include "../../headers/ze_platform.h"

internal i32 g_running = YES;

ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc)
{
	ZCFG_Init(cmdLine, argv, argc);
	return ZE_ERROR_NONE;
}

ze_external void ZE_Shutdown()
{
	g_running = NO;
}

ze_external i32 ZE_StartLoop()
{
	while (g_running)
	{
		Platform_PollEvents();
		Platform_Draw();
	}
	return 0;
}

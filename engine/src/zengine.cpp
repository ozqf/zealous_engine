#include "../../headers/zengine.h"
#include "../internal_headers/zengine_internal.h"

internal i32 g_running = YES;

ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc)
{
	return ZCFG_Init(cmdLine, argv, argc);
}

ze_external zErrorCode ZE_Init()
{
	ZAssets_Init();
	ZGen_Init();
	ZEmbedded_Init();
	ZScene_Init();
	ZAssets_PrintAll();
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
		ZScene_Draw();
	}
	return 0;
}

#include "../../headers/zengine.h"
#include "../internal_headers/zengine_internal.h"

internal i32 g_running = YES;
internal ZGame g_game = {};

ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc)
{
	return ZCFG_Init(cmdLine, argv, argc);
}

internal ZEngine ZE_BuildPlatformExport()
{
	ZEngine engine = {};
	engine.sentinel = ZE_SENTINEL;
	engine.scenes.AddScene = ZScene_CreateScene;
	engine.scenes.AddObject = ZScene_AddObject;
	return engine;
}

ze_external zErrorCode ZE_Init()
{
	ZAssets_Init();
	ZGen_Init();
	ZEmbedded_Init();
	ZScene_Init();
	ZAssets_PrintAll();
	g_game = ZGame_StubLinkup(ZE_BuildPlatformExport());
	if (g_game.sentinel == ZE_SENTINEL)
	{
		g_game.Init();
	}
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
		if (g_game.sentinel == ZE_SENTINEL)
		{
			g_game.Tick();
		}
		ZScene_Draw();
	}
	return 0;
}

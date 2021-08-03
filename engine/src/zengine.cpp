#include "../../headers/zengine.h"
#include "../internal_headers/zengine_internal.h"

internal i32 g_running = YES;
internal ZGame g_game = {};
internal ZEngine g_engine;

internal ZEngine ZE_BuildPlatformExport()
{
	ZEngine engine = {};
	engine.sentinel = ZE_SENTINEL;

	engine.scenes.AddScene = ZScene_CreateScene;
	engine.scenes.AddObject = ZScene_AddObject;
	engine.scenes.GetCamera = ZScene_GetCamera;
	engine.scenes.SetCamera = ZScene_SetCamera;
	engine.scenes.SetProjection = ZScene_SetProjection;

	engine.assets.AllocTexture = ZAssets_AllocTex;
	engine.assets.GetTexById = ZAssets_GetTexById;
	engine.assets.GetTexByName = ZAssets_GetTexByName;
	return engine;
}

ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc)
{
	g_engine = ZE_BuildPlatformExport();
	return ZCFG_Init(cmdLine, argv, argc);
}

ze_external ZEngine GetEngine()
{
	return g_engine;
}

ze_external zErrorCode ZE_Init()
{
	// step 1
	ZDebug_Init_1();
	ZAssets_Init();
	ZGen_Init();
	ZEmbedded_Init();
	ZScene_Init();
	ZInput_Init(&g_engine.input);

	// step 2
	ZDebug_Init_2();

	ZAssets_PrintAll();

	// link to app DLL
	g_game = ZGame_StubLinkup(g_engine);
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

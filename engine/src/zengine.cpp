#include "../../headers/zengine.h"
#include "../internal_headers/zengine_internal.h"

internal i32 g_running = YES;
internal ZGame g_game = {};
internal ZGameDef g_gameDef = {};
internal ZEngine g_engine;
internal i32 g_bSingleFrame = NO;
ze_internal i32 g_frameNumber = 0;

ze_internal i32 g_targetFPS = 0;
ze_internal f32 g_targetDelta = 0;

ZCMD_CALLBACK(Exec_Exit)
{
	g_running = false;
}

internal ZEngine ZE_BuildPlatformExport()
{
	ZEngine engine = {};
	engine.sentinel = ZE_SENTINEL;
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

ze_external ZGameDef GetGameDef()
{
	return g_gameDef;
}

internal void ZE_LinkToGame(ZGame_LinkupFunction gameLink)
{
	g_game = {};
	g_gameDef = {};
	zErrorCode err = gameLink(g_engine, &g_game, &g_gameDef);
	if (g_gameDef.targetFramerate > 0)
	{
		g_targetFPS = g_gameDef.targetFramerate;
		g_targetDelta = 1.f / (f32)g_targetFPS;
	}
	if (g_game.sentinel == ZE_SENTINEL)
	{
		if (g_game.Init != NULL)
		{
			g_game.Init();
		}
	}
}

ze_external i32 GetSingleFrameMode()
{
	return g_bSingleFrame;
}

ze_external zErrorCode ZEngine_Init(ZSystem systemFunctions, ZGame_LinkupFunction gameLink)
{
	i32 tokenIndex = ZCFG_FindParamIndex("--single", "--single", 0);
	if (tokenIndex > 0)
	{
		g_bSingleFrame = YES;
	}

	// grab everyone's export functions
	g_engine.system = systemFunctions;
	g_engine.assets = ZAssets_RegisterFunctions();
	g_engine.scenes = ZScene_RegisterFunctions();
	g_engine.input = ZInput_RegisterFunctions();
	g_engine.textCommands = ZCmdConsole_RegisterFunctions();
	ZCFG_RegisterFunctions();

	// initialise now that game struct is ready
	ZCmdConsole_Init();
	ZCmdConsole_RegisterInternalCommand("exit", "Close application.", Exec_Exit);
	ZCmdConsole_RegisterInternalCommand("quit", "Close application.", Exec_Exit);

	ZCFG_RegisterTextCommands();

	ZDebug_Init_1();
	ZAssets_Init();
	// ZGen_Init();
	ZEmbedded_Init();

	// further init that requires services to be running
	ZDebug_Init_2();
	ZCmdConsole_Init_b();

	ZAssets_PrintAll();

	// link to app DLL
	ZE_LinkToGame(gameLink);
	return ZE_ERROR_NONE;
}

ze_external void ZEngine_BeginShutdown()
{
	g_running = NO;
}

ze_external i32 ZE_StartLoop()
{
	// f64 targetInterval = 1.f / 60.f;
	f64 lastTickTime = Platform_QueryClock();
	while (g_running)
	{
		f64 now = Platform_QueryClock();
		f64 diff = now - lastTickTime;
		// printf("Frame: diff: %.8f, now: %.8f, then: %.8f\n",
		// 	diff, now, lastTickTime);
		if (diff < g_targetDelta)
		{
			// if we have loads of time until the next frame, sleep
			if (g_targetDelta / diff > 2)
			{
				Platform_Sleep(1);
			}
			continue;
		}
		lastTickTime = now;
		g_frameNumber += 1;
		ZEFrameTimeInfo info = {};
		info.frameRate = g_targetFPS;
		info.frameNumber = g_frameNumber;
		info.interval = diff;
		ZCmdConsole_Execute();
		Platform_PollEvents();
		if (g_game.sentinel == ZE_SENTINEL && g_game.Tick != NULL)
		{
			g_game.Tick(info);
		}
		ZScene_PostFrameTick();
		ZScene_Draw();

		if (g_bSingleFrame)
		{
			ZR_Screenshot("screenshot.png");
			g_running = false;
		}
	}
	// shutdown
	Window_Shutdown();
	return 0;
}

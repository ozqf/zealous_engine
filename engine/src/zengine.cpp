/*
Core initialisation and frameloop.
*/
#include "../../headers/zengine.h"
#include "../internal_headers/zengine_internal.h"

ze_internal i32 g_running = YES;
ze_internal ZGame g_game = {};
ze_internal ZGameDef g_gameDef = {};
ze_internal ZEngine g_engine;
ze_internal i32 g_bSingleFrame = NO;
ze_internal frameInt g_frameNumber = 0;

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

ze_external frameInt ZEngine_GetFrameNumber()
{
	return g_frameNumber;
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

ze_external zErrorCode ZEngine_Init(ZSystem systemFunctions, ZFileIO ioFunctions, ZGame_LinkupFunction gameLink)
{
	i32 tokenIndex = ZCFG_FindParamIndex("--single", "--single", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		g_bSingleFrame = YES;
	}

	// grab everyone's export functions
	g_engine.system = systemFunctions;
	g_engine.io = ioFunctions;
	g_engine.cfg = ZCFG_RegisterFunctions();
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
	ZInput_Init();

	// further init that requires services to be running
	ZDebug_Init_2();
	ZCmdConsole_Init_b();

	// ZAssets_PrintAll();

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

		// normal - tick game
		if (ZR_GetGraphicsTestMode() == 0)
		{
			if (g_game.sentinel == ZE_SENTINEL
			&& g_game.Tick != NULL)
			{
				f64 gameTickStart = Platform_QueryClock();
				g_game.Tick(info);
				f64 gameTickEnd = Platform_QueryClock();
				// printf("Game tick %.3f\n", (gameTickEnd - gameTickStart) * 1000);
			}

			ZScene_PostFrameTick();

			// ZRenderer renderer
			if (g_gameDef.flags & GAME_DEF_FLAG_MANUAL_RENDER && g_game.Draw != NULL)
			{
				ZRenderer r = {};
				r.ClearFrame = ZR_ClearFrame;
				r.ExecuteCommands = ZR_ExecuteCommands;
				g_game.Draw(r);
        		Platform_SubmitFrame();
			}
			else
			{
				ZScene_Draw();
			}
		}
		// ignore game and scene manager, call render test.
		else
		{
			f64 testStart = Platform_QueryClock();
        	ZR_DrawTest();
        	Platform_SubmitFrame();
        	f64 testEnd = Platform_QueryClock();
        	// printf("Draw test time %.3fms\n",
        	    // (testEnd - testStart) * 1000.f);
		}
		
		if (g_bSingleFrame)
		{
			// TODO: For some reason in single mode frame buffer may still be empty at this point.
			// so if we take a screenshot it will be completely blank
			// tick one frame and then it will be filled.
			if (g_frameNumber < 2)
			{
				printf("Single frame tick\n");
				continue;
			}
			// printf("Single frame sleep - 1000\n");
			// Platform_Sleep(1000);
			ZR_Screenshot("screenshot.png");
			g_running = false;
		}
	}
	
	Window_Shutdown();
	return 0;
}

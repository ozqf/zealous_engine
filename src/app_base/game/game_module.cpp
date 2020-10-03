#include "game_internal.h"
#include "../../sys_events.h"

extern "C" i32 Game_Init(ZE_FatalErrorFunction fatalFunc)
{
	printf("GAME - init\n");
	ZE_SetFatalError(fatalFunc);

	void* ptr = NULL;

	// prepare alloc track
	g_mallocs = COM_InitMallocList(g_mallocItems, GAME_MAX_MALLOCS);

	// allocate sim
	i32 entBytes = Sim_CalcEntityArrayBytes(GAME_MAX_ENTS);
	SimEntity* ents = (SimEntity*)COM_Malloc(&g_mallocs, entBytes, 0, "Sim Ents");
	Sim_Init(fatalFunc, "Server", &g_sim, ents, GAME_MAX_ENTS);
	Sim_Reset(&g_sim);
	
	// Buffers for simulation I/O
	i32 pendingCmdBytes = MegaBytes(1);
	
	ptr = COM_Malloc(&g_mallocs, pendingCmdBytes, 0, "Sim Input");
	g_gameBuf.a = Buf_FromBytes((u8*)ptr, pendingCmdBytes);

	ptr = COM_Malloc(&g_mallocs, pendingCmdBytes, 0, "Sim Output");
	g_gameBuf.b = Buf_FromBytes((u8*)ptr, pendingCmdBytes);

	// sub modules
	CL_Init(fatalFunc, App_GetAssetDB());
	GSV_Init(fatalFunc);

	printf("--- Game Init Voxel World test ---\n");
	VW_Test();
	VWError err = VW_AllocChunk(16, &g_chunk);
	if (err != VW_ERROR_NONE)
	{
		printf("Error %d allocating voxel world chunk\n", err);
	}
	
	return ZE_ERROR_NONE;
}

internal i32 Game_StartGameSession(
	const char* mapName,
	const i32 appSessionMode,
	const i32 gameRules)
{
	g_gameBuf.a.Clear(NO);
	g_gameBuf.b.Clear(NO);
	// setup sim
	Sim_Reset(&g_sim);
	g_sim.flags |= SIM_SCENE_BIT_IS_SERVER;
	g_sim.flags |= SIM_SCENE_BIT_IS_CLIENT;
	g_sim.gameRules = gameRules;
	APP_PRINT(128, "GAME - start rules %d\n", gameRules);
	Sim_LoadMapFile(&g_sim, mapName, NO);
	// start sub-modules
	CLG_Start(&g_sim);
	GSV_Start(&g_sim);
	i32 localPlayerId = SV_CreateLocalPlayer(&g_sim, g_gameBuf.GetWrite());
	CL_RegisterLocalPlayer(&g_sim, localPlayerId);
	return ZE_ERROR_NONE;
}

extern "C" i32 Game_Start(const char* mapName, const i32 appSessionMode)
{
	return Game_StartGameSession(mapName, appSessionMode, SIM_GAME_RULES_SURVIVAL);
}

extern "C" i32 Game_StartTitle()
{
	return Game_StartGameSession("0", APP_SESSION_TYPE_SINGLE_PLAYER, SIM_GAME_RULES_NONE);
}

extern "C" i32 Game_Stop()
{
	CL_Stop();
	GSV_Stop();
	Sim_Reset(&g_sim);
	return ZE_ERROR_NONE;
}

internal void Game_ReadSystemEvents(ZEBuffer* sysEvents, timeFloat delta)
{
	g_systemEventTicks++;
	u8* read = sysEvents->start;
	u8* end = sysEvents->cursor;
	while (read < end)
	{
		SysEvent* ev = (SysEvent*)read;
		ErrorCode err = Sys_ValidateEvent(ev);
		if (err != ZE_ERROR_NONE)
		{
			printf("GAME - sys event read error %d\n", err);
			return;
		}
		read += ev->size;
		// read event
		if (ev->type == SYS_EVENT_INPUT)
		{
			SysInputEvent* input = (SysInputEvent*)ev;
			CL_ReadInputEvent(input, g_systemEventTicks);
		}
	}
}

extern "C" void Game_Tick(ZEBuffer* sysEvents, ZEBuffer* soundOutput, timeFloat delta)
{
	Game_ReadSystemEvents(sysEvents, delta);
	CL_PreTick(&g_sim, &g_gameBuf, delta);
	SV_PreTick(&g_sim, &g_gameBuf, delta);
	
	g_gameBuf.Swap();
	g_gameBuf.GetWrite()->Clear(NO);
	Sim_Tick(&g_sim, g_gameBuf.GetRead(), g_gameBuf.GetWrite(), soundOutput, delta);

	CL_PostTick(&g_sim, &g_gameBuf, delta);
	SV_PostTick(&g_sim, &g_gameBuf, delta);
}

extern "C" Transform Game_GetCamera()
{
	return CL_GetCamera(&g_sim);
}

extern "C" void Game_WriteDrawFrame(ZRViewFrame* frame)
{
	CL_WriteDrawFrame(&g_sim, frame);
}

extern "C" void Game_ToggleDrawFlag(const char* name)
{
	if (!ZE_CompareStrings(name, "AABB"))
	{
		printf("Toggle draw flag %s\n", name);
		CL_ToggleDrawFlag(CL_DEBUG_FLAG_DRAW_LOCAL_SERVER);
		return;
	}
}

extern "C" void Game_ResetDrawFlags()
{
	CL_ClearDebugFlags();
}

extern "C" void Game_ClearInputActions()
{
	CL_ClearActionInputs();
}

extern "C" void Game_KillPlayers()
{
	SV_KillPlayer();
}

extern "C" void Game_WriteSave(const char* fileName, ZEFileIO files)
{
	printf("Game - write save to %s\n", fileName);
	i32 handle = files.OpenFile(fileName, NO);
	ZE_ASSERT(handle > 0, "Game failed to open file to write");
	ZE_CREATE_STACK_BUF(buf, 512)
	char* data = "this is some data";
	buf.WriteString(data);
	files.WriteToFile(handle, buf.start, buf.Written());
	files.CloseFile(handle);
}

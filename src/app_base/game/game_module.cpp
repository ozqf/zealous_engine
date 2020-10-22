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
	SV_Init(fatalFunc, App_GetAssetDB());

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
	g_sim.info.flags |= SIM_SCENE_BIT_IS_SERVER;
	g_sim.info.flags |= SIM_SCENE_BIT_IS_CLIENT;
	g_sim.info.gameRules = gameRules;
	APP_PRINT(128, "GAME - start rules %d\n", gameRules);
	Sim_LoadMapFile(&g_sim, mapName, NO);
	// start sub-modules
	CL_Start(&g_sim);
	SV_Start(&g_sim);
	i32 localPlayerId = SV_CreateLocalPlayer(&g_sim, g_gameBuf.GetWrite());
	CL_RegisterLocalPlayer(&g_sim, localPlayerId);
	return ZE_ERROR_NONE;
}

internal i32 Game_ResumeSession(ZEBuffer* saveData, SimSaveFileInfo* saveInfo)
{

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
	if (!ZStr_Compare(name, "AABB"))
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

extern "C" void Game_DumpSessionInfo()
{
	SimScene* sim = &g_sim;
	printf("=== GAME INFO ===\n");
	printf("--- Ents (%d max)\n", sim->info.maxEnts);
	i32 activeEnts = 0;
	for (i32 i = 0; i < sim->info.maxEnts; ++i)
	{
		if (sim->data.ents[i].status != SIM_ENT_STATUS_FREE)
		{
			activeEnts++;
		}
	}
	printf("\t%d active ents\n", activeEnts);

	printf("%d max Players\n", sim->info.maxPlayers);
}

internal i32 Game_StageSaveFile(
	const char* fileName, ZEFileIO files, i32* handle, ZEBuffer* b, SimSaveFileInfo* fi)
{
	*handle = 0;
	*handle = files.StageFile(fileName, NO, b);
	if (*handle == 0) { return ZE_ERROR_NONE; }
	// check magic number
	u8* read = b->GetAtOffset(7);
	if (*read != NULL)
	{
		printf("Bad magic string terminator %d\n", *read);
		files.CloseFile(*handle);
		return ZE_ERROR_DESERIALISE_FAILED;
	}
	if (ZStr_Compare((char*)b->start, SIM_SAVE_MAGIC_STRING) != 0)
	{
		printf("Bad magic string %s in save\n", (char*)b->start);
		files.CloseFile(*handle);
		return ZE_ERROR_DESERIALISE_FAILED;
	}
	read++;
	*fi = *((SimSaveFileInfo*)read);
	if (fi->sentinel != SIM_SAVE_SENTINEL)
	{
		printf("Bad sentinel in SimSaveFileInfo\n");
		files.CloseFile(*handle);
		return ZE_ERROR_DESERIALISE_FAILED;
	}

	return ZE_ERROR_NONE;
}

extern "C" void Game_ScanSaveFile(const char* fileName, ZEFileIO files)
{
	ZEBuffer b;
	SimSaveFileInfo fi = {};
	i32 file = 0;
	
	ErrorCode err = Game_StageSaveFile(fileName, files, &file, &b, &fi);
	if (err != ZE_ERROR_NONE)
	{
		printf("Error %d staging save file %s\n", err, fileName);
		return;
	}

	printf("Sentinel: 0X%X\n", fi.sentinel);
	printf("info at %d, %d ents at %d. %d players at %d. Read (%dB) at %d. Write (%dB) at %d\n",
		fi.infoOffset,
		fi.numEnts, fi.entsOffset,
		fi.numPlayers, fi.players,
		fi.numReadBytes, fi.read,
		fi.numWriteBytes, fi.write);
	SimSceneInfo* info = (SimSceneInfo*)b.GetAtOffset(fi.infoOffset);
	printf("SV %d bytes at %d\n", fi.numServerBytes, fi.server);
	printf("CL %d bytes at %d\n", fi.numClientBytes, fi.client);
	printf("Gravity: %.3f, %.3f, %.3f\n", info->gravity.x, info->gravity.y, info->gravity.z);

	files.CloseFile(file);
}

extern "C" void Game_LoadSave(const char* fileName, ZEFileIO files)
{
	ZEBuffer b;
	SimSaveFileInfo fi = {};
	i32 file = 0;
	
	ErrorCode err = Game_StageSaveFile(fileName, files, &file, &b, &fi);
	if (err != ZE_ERROR_NONE)
	{
		printf("Error %d staging save file %s\n", err, fileName);
		return;
	}

}

extern "C" void Game_WriteSave(const char* fileName, ZEFileIO files)
{
	SimScene* sim = &g_sim;
	i32 sentinel = SIM_SAVE_SENTINEL;
	printf("Game - write save to %s\n", fileName);
	printf("\t%d ents, %d players\n", sim->info.maxEnts, sim->info.maxPlayers);
	i32 handle = files.OpenFile(fileName, NO);
	ZE_ASSERT(handle > 0, "Game failed to open file to write");

	ZE_CREATE_STACK_BUF(temp, 512)
	temp.WriteString(SIM_SAVE_MAGIC_STRING);
	// magic number - ignore null terminator
	files.WriteToFile(handle, temp.start, temp.Written());

	// file header after magic number
	SimSaveFileInfo fileInfo = {};
	fileInfo.sentinel = SIM_SAVE_SENTINEL;
	// record position for save info
	i32 offsetTablePos = files.FilePosition(handle);
	// pad space for save info
	files.WritePadding(handle, sizeof(SimSaveFileInfo), 0xFF);
	// Sim info
	fileInfo.infoOffset = files.FilePosition(handle);
	files.WriteToFile(handle, (u8*)&sim->info, sizeof(SimSceneInfo));

	// sentinel
	files.WriteToFile(handle, (u8*)&sentinel, sizeof(i32));
	////////////////////////////////////////////////////////
	// Ents
	fileInfo.entsOffset = files.FilePosition(handle);
	for (i32 i = 0; i < sim->info.maxEnts; ++i)
	{
		SimEntity* ent = &sim->data.ents[i];
		if (ent->status == SIM_ENT_STATUS_FREE) { continue; }
		files.WriteToFile(handle, (u8*)ent, sizeof(SimEntity));
		fileInfo.numEnts++;
	}

	// sentinel
	files.WriteToFile(handle, (u8*)&sentinel, sizeof(i32));
	////////////////////////////////////////////////////////
	// Players
	fileInfo.players = files.FilePosition(handle);
	for (i32 i = 0; i < sim->info.maxPlayers; ++i)
	{
		SimPlayer* plyr = &sim->data.players[i];
		
		if (plyr->state == SIM_PLAYER_STATE_NONE) { continue; }
		files.WriteToFile(handle, (u8*)plyr, sizeof(SimPlayer));
		fileInfo.numPlayers++;
	}
	
	////////////////////////////////////////////////////////
	// Event buffers
	
	// sentinel
	files.WriteToFile(handle, (u8*)&sentinel, sizeof(i32));
	ZEBuffer* cmdRead = g_gameBuf.GetRead();
	if (cmdRead->Written() > 0)
	{
		fileInfo.read = files.FilePosition(handle);
		fileInfo.numReadBytes = cmdRead->Written();
		files.WriteToFile(handle, cmdRead->start, cmdRead->Written());
	}
	
	// sentinel
	files.WriteToFile(handle, (u8*)&sentinel, sizeof(i32));
	ZEBuffer* cmdWrite = g_gameBuf.GetWrite();
	if (cmdWrite->Written() > 0)
	{
		fileInfo.write = files.FilePosition(handle);
		fileInfo.numWriteBytes = cmdWrite->Written();
		files.WriteToFile(handle, cmdWrite->start, cmdWrite->Written());
	}
	
	////////////////////////////////////////////////////////
	// Local Server
	SV_Save(sim, &fileInfo, handle, files);
	
	////////////////////////////////////////////////////////
	// Local Client
	CL_Save(sim, &fileInfo, handle, files);

	// patch in header
	files.WriteToFileAtOffset(handle, (u8*)&fileInfo, sizeof(SimSaveFileInfo), offsetTablePos);
	// printf("%d bytes in cmd read\n%d bytes in cmd write\n",
	// 	cmdRead->Written(), cmdWrite->Written());
	// Sim_DumpCommandBuffer(sim, cmdRead);
	// Sim_DumpCommandBuffer(sim, cmdWrite);

	files.CloseFile(handle);
}

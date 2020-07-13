#include "game_internal.h"
#include "../../sys_events.h"

extern "C" i32 Game_Init()
{
	printf("GAME - init\n");
	void* ptr = NULL;

	// prepare alloc track
	g_mallocs = COM_InitMallocList(g_mallocItems, GAME_MAX_MALLOCS);

	// allocate sim
	i32 entBytes = Sim_CalcEntityArrayBytes(GAME_MAX_ENTS);
	SimEntity* ents = (SimEntity*)COM_Malloc(&g_mallocs, entBytes, "Sim Ents");
	Sim_Init("Server", &g_sim, ents, GAME_MAX_ENTS);
	Sim_Reset(&g_sim);
	
	// Buffers for simulation I/O
	i32 pendingCmdBytes = MegaBytes(1);
	
	ptr = COM_Malloc(&g_mallocs, pendingCmdBytes, "Sim Input");
	g_gameBuf.a = Buf_FromBytes((u8*)ptr, pendingCmdBytes);

	ptr = COM_Malloc(&g_mallocs, pendingCmdBytes, "Sim Output");
	g_gameBuf.b = Buf_FromBytes((u8*)ptr, pendingCmdBytes);

	// scene render
	g_rendCfg = {};
	g_rendCfg.extraLightsMax = 16;
	g_rendCfg.worldLightsMax = 16;
	g_rend = CLR_Create(App_GetAssetDB(), 128);

	// sub modules
	CL_Init();
	GSV_Init();
	
	return ZE_ERROR_NONE;
}

extern "C" i32 Game_Start()
{
	// setup sim
	Sim_Reset(&g_sim);
	Sim_LoadStaticScene(&g_sim, 0);
	// start sub-modules
	CLG_Start();
	GSV_Start(&g_sim);
	CL_RegisterLocalPlayer(GSV_CreateLocalPlayer(&g_sim));
	return ZE_ERROR_NONE;
}

extern "C" i32 Game_Stop()
{
	CL_Stop();
	GSV_Stop();
	return ZE_ERROR_NONE;
}

internal void Game_ReadSystemEvents(ZEByteBuffer* sysEvents, timeFloat delta)
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

extern "C" i32 Game_Tick(ZEByteBuffer* sysEvents, timeFloat delta)
{
	//printf("GTICK - Reading %d bytes\n", sysEvents->Written());
	Game_ReadSystemEvents(sysEvents, delta);
	CL_PreTick(delta);
	
	g_gameBuf.Swap();
	g_gameBuf.GetWrite()->Clear(NO);
	Sim_Tick(&g_sim, g_gameBuf.GetRead(), g_gameBuf.GetWrite(), delta);

	#if 0
	g_sim.timeInAABBSearch = 0;
    for (i32 i = 0; i < g_sim.maxEnts; ++i)
    {
        SimEntity* ent = &g_sim.ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
		Game_TickEntity(&g_sim, ent, delta);
        // make sure previous positions are updated
        ent->body.previousPos = ent->body.t.pos;
    }
    g_sim.tick++;
    g_sim.time += delta;
	#endif
	return ZE_ERROR_NONE;
}

extern "C" void Game_WriteDrawFrame(ZRViewFrame* frame)
{
	Transform cam = CL_GetDebugCamera();
	CLR_WriteDrawFrame(g_rend, frame, &g_sim, &cam, NULL, 0, g_rendCfg);
}

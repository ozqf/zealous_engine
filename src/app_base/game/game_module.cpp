#include "game_internal.h"
#include "../../sys_events.h"


extern "C" i32 Game_Init()
{
	printf("GAME - init\n");
	
    Transform_SetToIdentity(&g_camera);
    g_camera.pos.z = 10;
    g_camera.pos.y += 34;
    Transform_SetRotation(&g_camera, -(80.0f    * DEG2RAD), 0, 0);

	g_mallocs = COM_InitMallocList(g_mallocItems, GAME_MAX_MALLOCS);
	i32 entBytes = Sim_CalcEntityArrayBytes(GAME_MAX_ENTS);
	SimEntity* ents = (SimEntity*)COM_Malloc(&g_mallocs, entBytes, "Sim Ents");
	Sim_Init("Server", &g_sim, ents, GAME_MAX_ENTS);
	Sim_Reset(&g_sim);
	Sim_LoadStaticScene(&g_sim, 0);

	g_rendCfg = {};
	g_rendCfg.extraLightsMax = 16;
	g_rendCfg.worldLightsMax = 16;
	g_rend = CLR_Create(App_GetAssetDB(), 128);

	return ZE_ERROR_NONE;
}

internal void Game_ReadSystemEvents(ZEByteBuffer* sysEvents, timeFloat delta)
{
	u8* read = sysEvents->start;
	u8* end = sysEvents->cursor;
	while (read < end)
	{
		SysEvent* ev;
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
			
		}
	}
}

extern "C" i32 Game_Tick(ZEByteBuffer* sysEvents, timeFloat delta)
{
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
	return ZE_ERROR_NONE;
}

extern "C" void Game_WriteDrawFrame(ZRViewFrame* frame)
{
	CLR_WriteDrawFrame(g_rend, frame, &g_sim, &g_camera, NULL, 0, g_rendCfg);
}

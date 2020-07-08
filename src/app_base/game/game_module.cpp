#include "game_internal.h"


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
	return ZE_ERROR_NONE;
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
	
}

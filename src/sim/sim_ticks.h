#pragma once

#include "sim.h"
/*
Tick functions shared between client and server
*/

extern "C"
i32 Sim_TickSpawn(
    SimScene* sim, SimEntity* ent, f32 deltaTime)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        ent->flags &= ~SIM_ENT_FLAG_OUT_OF_PLAY;
        ent->tickType = ent->coreTickType;
        ent->body.t.scale = { 1, 1, 1 };
    }
    else
    {
        ent->flags |= SIM_ENT_FLAG_OUT_OF_PLAY;
        i32 totalWait = ent->timing.nextThink - ent->timing.lastThink;
        i32 progress = sim->tick - ent->timing.lastThink;
        
        f32 time = (f32)progress / (f32)totalWait;
        ent->body.t.scale.x = COM_LerpF32(0.01f, 0.5f, time);
        ent->body.t.scale.y = COM_LerpF32(50.0f, 1.0f, time);
        ent->body.t.scale.z = COM_LerpF32(0.01f, 0.5f, time);
    }
    return ZE_ERROR_NONE;
}

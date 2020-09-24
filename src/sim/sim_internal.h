#pragma once

// TODO: Remove: This include is purely due to requiring access to
// APP_LOG and APP_PRINT!
#include "../app_base/app.h"
#include "sim.h"
#include <math.h>

// Internal Sim module functions/data
internal SimEntity* Sim_FindEntityBySerialNumber(SimScene* scene, i32 serialNumber);
internal SimEntity* Sim_GetFreeReplicatedEntity(SimScene* scene, i32 newSerial);
internal SimEntity* Sim_GetFreeLocalEntity(SimScene* scene, i32 newSerial);
internal i32        Sim_FreeEntityBySerial(SimScene* scene, i32 serial);
internal i32        Sim_FindFreeSlot(SimScene* scene, i32 forLocalEnt);
internal i32        Sim_EnqueueCommand(SimScene* sim, u8* ptr);
internal i32        Sim_RecycleEntity(SimScene* sim, i32 entitySerialNumber);
internal i32        Sim_MarkEntityAsRemoved(SimScene* sim, i32 serialNumber);
internal void       Sim_WriteRemoveEntity(
    SimScene* sim, SimEntity* victim, SimEntity* attacker, i32 style, Vec3 dir, i32 deathIsDeterministic);
static void         SimEnt_MoveVsSolid(SimScene* sim, SimEntity* ent, Vec3 move);
internal i32        SimEnt_CheckTeamDiffer(i32 atkTeam, i32 victimTeam);
internal i32        SimRules_SpawnPlayer(SimScene* sim, SimPlayer* plyr);

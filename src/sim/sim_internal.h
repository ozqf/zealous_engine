#pragma once

// TODO: Remove: This include is purely due to requiring access to
// APP_LOG and APP_PRINT!
#include "../app/app.h"
#include "sim.h"
#include <math.h>

// Internal Sim module functions/data
internal SimEntity*
    Sim_FindEntityBySerialNumber(SimScene* scene, i32 serialNumber);
internal SimEntity*
    Sim_GetFreeReplicatedEntity(SimScene* scene, i32 newSerial);
internal SimEntity* Sim_GetFreeLocalEntity(SimScene* scene, i32 newSerial);
internal i32        Sim_FreeEntityBySerial(SimScene* scene, i32 serial);
internal i32        Sim_FindFreeSlot(SimScene* scene, i32 forLocalEnt);
internal i32        Sim_EnqueueCommand(SimScene* sim, u8* ptr);
internal i32        Sim_RecycleEntity(SimScene* sim, i32 entitySerialNumber);

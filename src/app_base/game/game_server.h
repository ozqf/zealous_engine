#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "game.h"
#include "../../sim/sim.h"

extern "C" void SV_Init(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* db);
extern "C" void SV_Start(SimScene* sim);
extern "C" void GSV_Stop();
extern "C" void SV_Save(SimScene* sim, SimSaveFileInfo* saveInfo, i32 file, ZEFileIO files);
extern "C" void SV_Resume(SimScene* sim, SimSaveFileInfo* saveInfo, ZEBuffer* saveData);

extern "C" i32 SV_CreateLocalPlayer(SimScene* sim, ZEBuffer* buf);
extern "C" void SV_PreTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta);
extern "C" void SV_PostTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta);
extern "C" void SV_KillPlayer();

#endif // GAME_SERVER_H
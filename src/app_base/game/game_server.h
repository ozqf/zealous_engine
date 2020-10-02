#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "game.h"
#include "../../sim/sim.h"

extern "C" void GSV_Init(ZE_FatalErrorFunction fatalFunc);
extern "C" void GSV_Start(SimScene* sim);
extern "C" void GSV_Stop();
extern "C" i32 SV_CreateLocalPlayer(SimScene* sim, ZEBuffer* buf);
extern "C" void SV_PreTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta);
extern "C" void SV_PostTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta);
extern "C" void SV_KillPlayer();

#endif // GAME_SERVER_H
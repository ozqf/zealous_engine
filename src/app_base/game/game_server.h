#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "game.h"
#include "../../sim/sim.h"

extern "C" void GSV_Init();
extern "C" void GSV_Start(SimScene* sim);
extern "C" void GSV_Stop();
extern "C" SimPlayer GSV_CreateLocalPlayer(SimScene* sim, ZEByteBuffer* buf);
extern "C" void SV_PreTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta);
extern "C" void SV_PostTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta);

#endif // GAME_SERVER_H
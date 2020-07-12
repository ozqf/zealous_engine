#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "game.h"
#include "../../sim/sim.h"

extern "C" void GSV_Init();
extern "C" void GSV_Start();
extern "C" void GSV_Stop();
extern "C" SimPlayer GSV_CreateLocalPlayer(SimScene* sim);
extern "C" void GSV_PreTick(timeFloat delta);
extern "C" void GSV_PostTick(timeFloat delta);

#endif // GAME_SERVER_H
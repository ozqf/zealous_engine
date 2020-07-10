#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include "game.h"

extern "C" void SV_Init();
extern "C" void SV_Start();
extern "C" void SV_Stop();
extern "C" void SV_PreTick(timeFloat delta);
extern "C" void SV_PostTick(timeFloat delta);

#endif // GAME_SERVER_H
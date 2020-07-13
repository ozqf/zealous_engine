#ifndef GAME_H
#define GAME_H

#include "../../ze_common/ze_common_full.h"
#include "../../zqf_renderer.h"

extern "C" i32 Game_Init();
extern "C" i32 Game_Start();
extern "C" i32 Game_Stop();
extern "C" void Game_Tick(ZEByteBuffer* sysEvents, timeFloat delta);
extern "C" void Game_WriteDrawFrame(ZRViewFrame* frame);

#endif // GAME_H
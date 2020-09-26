#ifndef GAME_H
#define GAME_H

#include "../../ze_common/ze_common_full.h"
#include "../../zqf_renderer.h"

extern "C" i32 Game_Init(ZE_FatalErrorFunction fatalFunc);
extern "C" i32 Game_Start(const char* mapName, const i32 appSessionMode);
extern "C" i32 Game_Stop();
extern "C" i32 Game_StartTitle();
extern "C" void Game_Tick(ZEByteBuffer* sysEvents, ZEByteBuffer* soundOutput, timeFloat delta);
extern "C" void Game_WriteDrawFrame(ZRViewFrame* frame);
extern "C" Transform Game_GetCamera();
extern "C" void Game_ToggleDrawFlag(const char* name);
extern "C" void Game_ResetDrawFlags();
extern "C" void Game_ClearInputActions();
extern "C" void Game_KillPlayers();

#endif // GAME_H
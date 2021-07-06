#ifndef GAME_H
#define GAME_H

#include "../../../headers/common/ze_common_full.h"
#include "../../zqf_renderer.h"

extern "C" i32 Game_Init(ZE_FatalErrorFunction fatalFunc);
extern "C" i32 Game_Start(const char* mapName, const i32 appSessionMode);
extern "C" i32 Game_Stop();
extern "C" i32 Game_StartTitle();
extern "C" void Game_Tick(ZEBuffer* sysEvents, ZEBuffer* soundOutput, timeFloat delta);
extern "C" void Game_WriteDrawFrame(ZRViewFrame* frame);
extern "C" Transform Game_GetCamera();
extern "C" void Game_ToggleDrawFlag(const char* name);
extern "C" void Game_ResetDrawFlags();
extern "C" void Game_ClearInputActions();
extern "C" void Game_WriteSave(const char* fileName, ZEFileIO files);

// debug related
extern "C" void Game_KillPlayers();
extern "C" void Game_DumpSessionInfo();
extern "C" void Game_ScanSaveFile(const char* fileName, ZEFileIO files);

#endif // GAME_H
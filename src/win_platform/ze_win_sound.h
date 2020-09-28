#ifndef ZE_WIN_SOUND_H
#define ZE_WIN_SOUND_H

#include "../ze_common/ze_common_full.h"

extern "C" ErrorCode Snd_Init();
extern "C" ErrorCode Snd_Shutdown();
extern "C" ErrorCode Snd_ParseCommandString(const char* str, const char** tokens, const i32 numTokens);

extern "C" i32 Snd_LoadSoundWavFile(char* name64, char* filePath);
extern "C" i32 Snd_LoadSoundRaw(char* name64, u8* data, i32 numBytes);
extern "C" void Snd_PlayQuick(i32 sampleIndex, Vec3 pos);
extern "C" void Snd_ExecCommands(ZEBuffer* buf);

#endif // ZE_WIN_SOUND_H
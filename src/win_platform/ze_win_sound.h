#ifndef ZE_WIN_SOUND_H
#define ZE_WIN_SOUND_H

#include "../ze_common/ze_common_full.h"

enum ZSoundEventType
{
    zPlaySound,
    zMoveSound
};

union ZSoundEventUnion
{
    struct ZPlaySound
    {
        u8 spatial;
	    f32 pos[3];
    };

    struct ZMoveSound
    {
        f32 pos[3];
    };
};

// An event that kicks off a sound source
struct ZSoundEvent
{
	i32 soundId;
    ZSoundEventType type;
    ZSoundEventUnion data;
};
/*
void SND_SetPlaySoundEvent(ZSoundEvent* ev, i32 soundId, u8 spatial)
{
    ev->type = zPlaySound;
}

void SND_SetMoveSoundEvent(ZSoundEvent* ev)
{
    
}
*/

extern "C" u8 Snd_Init();
extern "C" u8 Snd_Shutdown();
extern "C" u8 Snd_ParseCommandString(const char* str, const char** tokens, const i32 numTokens);

#endif // ZE_WIN_SOUND_H
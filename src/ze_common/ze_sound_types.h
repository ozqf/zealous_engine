#ifndef ZE_SOUND_TYPES_H
#define ZE_SOUND_TYPES_H

#include "ze_common.h"

enum ZSoundEventType
{
    PlaySound,
    MoveSound
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

#endif // ZE_SOUND_TYPES_H
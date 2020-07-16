#ifndef ZE_SOUND_TYPES_H
#define ZE_SOUND_TYPES_H

#include "ze_common.h"

///////////////////////////////////////
// Commands
///////////////////////////////////////
#define ZS_FLAG_3D (1 << 0)

enum ZSoundCommandType
{
    PlaySound,
    MoveSound,
	KillSound
};

union ZSoundCommandUnion
{
    struct play
    {
		i32 userEventId;
        i32 flags;
	    f32 pos[3];
    };

    struct move
    {
        f32 pos[3];
    };
};

// Holds instructions to the sound system.
struct ZSoundCommand
{
	// caller's Identifier for this sound source.
	i32 userId;
	// bits to group sound events together
	i32 groupMask;
    ZSoundCommandType type;
    ZSoundCommandUnion data;
};

struct ZSndCreate
{
	i32 userEventId;
	char* sourceFileName64;
	
};

struct ZSoundInstance
{
    i32 userId;
    i32 groupMask;
};

struct ZSoundScene
{
    i32 nextEventId;

};

#endif // ZE_SOUND_TYPES_H
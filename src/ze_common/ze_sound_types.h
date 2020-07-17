#ifndef ZE_SOUND_TYPES_H
#define ZE_SOUND_TYPES_H

#include "ze_common.h"

///////////////////////////////////////
// Commands
///////////////////////////////////////
#define ZS_FLAG_3D (1 << 0)

#define ZSOUND_EVENT_NONE 0
#define ZSOUND_EVENT_PLAY 1
#define ZSOUND_EVENT_MOVE 2
#define ZSOUND_EVENT_KILL 3
#define ZSOUND_EVENT_SET_LISTENER 4

union ZSoundCommandUnion
{
    struct
    {
		i32 soundEventType;
        i32 flags;
	    Vec3 pos;
    } play;

    struct
    {
        Vec3 pos;
    } move;
    Transform listener;
};

// Holds instructions to the sound system.
struct ZSoundCommand
{
	// caller's Identifier for this sound source.
	i32 userId;
	// bits to group sound events together
	i32 groupMask;
    i32 type;
    ZSoundCommandUnion data;
};

struct ZSndCreate
{
	i32 userEventId;
	char* sourceFileName64;
	
};

struct ZSoundEventType
{
    i32 index;
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
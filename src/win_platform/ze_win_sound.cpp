
#include "../../lib/fmod/fmod_studio.hpp"
#include "../../lib/fmod/fmod.hpp"

#include "ze_win_sound.h"
#include "../ze_common/ze_common_full.h"

/////////////////////////////////////////////////////////////////////
// Internal data structures
/////////////////////////////////////////////////////////////////////

// A playable sound sample loaded by FMOD
struct ZSound
{
	i32 id;
	char name[64];
	FMOD::Sound* handle;    // if handle is null, this sound is unassigned
};

// An entity in the sound 'scene'
struct ZSoundSource
{
	i32 id;
	i32 soundId;            // id of -1 == unused
	f32 pos[3];
	f32 tick;
};


/*
Sound notes:

-- Types of sounds --
> Spatial sounds: Anything in the 'world' that has a position and direction
    eg Gunfire, enemies, explosions.
> Ambient: Sounds which don't necessarily have direction but still vary
    by volume depending on position. eg wind, flowing water etc.
> Non-spatial one-off:
    eg UI, own character sounds, item pickups etc.
> Non-spatial, looping, constant volume:
    eg Music

*/

/////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////

// https://www.fmod.com/docs/api/content/generated/FMOD_MODE.html
// flag to tell fmod to read name_or_data params as data, not file names
// In open memory mode fmod will duplicate the data so it can be freed
// once loaded
// bit field FMOD_OPENMEMORY
// this flag does the same as the above but does NOT copy data,
// so data must be manually handled. requires Sound::release to free
// bit field FMOD_OPENMEMORY_POINT

internal FMOD::Sound* gsnd_soundHandle;
internal FMOD::Channel* gsnd_channel;

internal ZSound g_sounds[128];
internal i32 g_nextSoundId = 0;
internal ZSoundSource g_sources[128];

internal FMOD::Channel* g_channels[32];

internal FMOD::System* sys = NULL;

internal u8 g_testSoundLoaded = 0;

//internal ZSound g_sounds[128];

internal u8 Snd_Play2(ZSoundEvent* ev)
{
    //gsnd_channel.set3DAttributes()
    return 1;
}

#if 0 // FMOD Studio - broken atm
u8 Snd_Init()
{
    FMOD_RESULT result;
    FMOD::Studio::System* system = NULL;

    result = FMOD::Studio::System::create(&system); // Create the Studio System object.
    if (result != FMOD_OK)
    {
        return 1;
    }

    // Initialize FMOD Studio, which will also initialize FMOD Low Level
    result = system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        //printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        return 1;
    }

    result = FMOD::Studio::System::createSound("Frenzy_Beam_Loop.wav", FMOD_DEFAULT, NULL, &g_soundHandle);

    return 0;
}
#endif

// Returns id of new sound
internal i32 Snd_LoadSound(char* name64, u8* data, i32 numBytes)
{
    printf("SOUND Creating sound, %d bytes from %d\n", numBytes, (u32)data);
    // test example, commented out until I've made a decision on asset storage/version control
    FMOD_CREATESOUNDEXINFO info = {};
    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    info.length = numBytes;
    FMOD_RESULT result = sys->createSound((const char*)data, FMOD_OPENMEMORY, &info, &gsnd_soundHandle);
    printf(" create result: %d\n", result);
    if (result == FMOD_OK)
    {
        g_testSoundLoaded = 1;
    }
    return 1;
}

#if 1 // Low level API only
extern "C" ErrorCode Snd_Init()
{
    printf("SOUND Init\n");
    FMOD_RESULT result;
    //FMOD::System* system = NULL;

    result = FMOD::System_Create(&sys); // Create the Studio System object.
    if (result != FMOD_OK)
    {
        return ZE_ERROR_UNKNOWN;
    }
    gsnd_channel->setVolume(0.5f);
    // Initialize FMOD Low Level
    result = sys->init(512, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        //printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        return ZE_ERROR_UNKNOWN;
    }
#if 0
    // test example, commented out until I've made a decision on asset storage/version control
    char* name = "data/Frenzy_Beam_Loop.wav";
    printf("Loading test sound %s\n", name);
    result = sys->createSound(name, 0, NULL, &gsnd_soundHandle);
    result = sys->playSound(gsnd_soundHandle, NULL, false, &gsnd_channel);
#endif
    return ZE_ERROR_NONE;
}
#endif

extern "C" ErrorCode Snd_Shutdown()
{
    printf("SOUND Shutting down\n");
    FMOD_RESULT result;
    if (g_testSoundLoaded == 1)
    {
        result = gsnd_soundHandle->release();
        printf("SND release result: %d\n", result);
    }
    return 1;
}

extern "C" ErrorCode Snd_ParseCommandString(const char* str, const char** tokens, const i32 numTokens)
{
    if (ZE_CompareStrings(tokens[0], "HELP") == 0)
    {
        printf("SND TEST - play test sound\n");
    }
    if (ZE_CompareStrings(tokens[0], "SND") == 0)
    {
        if (numTokens == 2 && ZE_CompareStrings(tokens[1],"TEST") == 0)
        {
            if (g_testSoundLoaded == 0)
            {
                printf("Cannot play sound test - no sounds loaded");
                return 1;
            }
            FMOD_RESULT result = sys->playSound(gsnd_soundHandle, NULL, false, &gsnd_channel);
            printf("  play result: %d\n", result);
        }
        return 1;
    }
    if (ZE_CompareStrings(tokens[0], "VERSION") == 0)
	{
		printf("SOUND Built %s: %s\n", __DATE__, __TIME__);
		return 0;
	}
    return 0;
}

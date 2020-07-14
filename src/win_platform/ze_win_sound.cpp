
#include "../../lib/fmod/fmod_studio.hpp"
#include "../../lib/fmod/fmod.hpp"

#include "ze_win_sound.h"
#include "../ze_common/ze_common_full.h"

#define SND_MAX_SOUND_HANDLES 256
#define SND_MAX_CHANNELS 32

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

//internal FMOD::Sound* gsnd_soundHandle;
internal FMOD::Sound* g_soundHandles[SND_MAX_SOUND_HANDLES];
internal i32 g_nextSoundHandle = 0;
internal FMOD::Channel* gsnd_channel;
internal FMOD::Channel* g_channels[SND_MAX_CHANNELS];
internal FMOD::System* sys = NULL;

internal ZSound g_sounds[128];
internal i32 g_nextSoundId = 0;
internal ZSoundSource g_sources[128];

//internal ZSound g_sounds[128];

////////////////////////////////////////////////////////
// Init/Shutdown
////////////////////////////////////////////////////////
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
    gsnd_channel->setVolume(0.4f);
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
    //FMOD_RESULT result;
    for (i32 i = g_nextSoundHandle - 1; i >= 0; --i)
    {
        g_soundHandles[i]->release();
    }
    return ZE_ERROR_NONE;
}

////////////////////////////////////////////////////////
// handles
////////////////////////////////////////////////////////
internal FMOD::Sound** Snd_GetFreeSoundHandle(i32* resultIndex)
{
    *resultIndex = g_nextSoundHandle;
    return &g_soundHandles[g_nextSoundHandle++];
}

////////////////////////////////////////////////////////
// Load Sounds
////////////////////////////////////////////////////////
extern "C" i32 Snd_LoadSoundWavFile(char* name64, char* filePath)
{
    //char* name = "data/Frenzy_Beam_Loop.wav";
    i32 index = -1;
    FMOD::Sound** handlePtr = Snd_GetFreeSoundHandle(&index);
    printf("Loading test sound %s to index %d\n", filePath, index);
    FMOD_RESULT result = sys->createSound(filePath, 0, NULL, handlePtr);
    if (result == FMOD_OK)
    {
        //result = sys->playSound(handle, NULL, false, &gsnd_channel);
        return index;
    }
    else
    {
        printf("SND error %d loading file %s\n", result, filePath);
        return -1;
    }
}

// Returns id of new sound
extern "C" i32 Snd_LoadSoundRaw(char* name64, u8* data, i32 numBytes)
{
    printf("SOUND Creating sound, %d bytes from %d\n", numBytes, (u32)data);
    // test example, commented out until I've made a decision on asset storage/version control
    FMOD_CREATESOUNDEXINFO info = {};
    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    info.length = numBytes;

    i32 index;
    FMOD::Sound** handlePtr = Snd_GetFreeSoundHandle(&index);
    FMOD_RESULT result = sys->createSound((const char*)data, FMOD_OPENMEMORY, &info, handlePtr);
    printf(" create result: %d\n", result);
    if (result == FMOD_OK)
    {
        return index;
    }
    return -1;
}

////////////////////////////////////////////////////////
// Play
////////////////////////////////////////////////////////
extern "C" void Snd_PlayQuick(i32 sampleIndex)
{
    if (sampleIndex < 0 || sampleIndex >= g_nextSoundHandle)
    {
        printf("\tsound index %d out of bounds\n", sampleIndex);
        return;
    }
    //printf("Play sound %d\n", sampleIndex);
    FMOD::Sound* handle = g_soundHandles[sampleIndex];
    if (handle == NULL)
    {
        printf("\tsound handle %d is null!\n", sampleIndex);
        return;
    }
    FMOD_RESULT result = sys->playSound(handle, NULL, false, &gsnd_channel);
    if (result != FMOD_OK)
    {
        printf("\tError %d playing sound %d\n", result, sampleIndex);
    }
}

extern "C" void Snd_ExecuteEvents(ZEByteBuffer buf)
{
    if (Buf_IsValid(&buf) == NO) { return; }
    if (buf.Written() == 0) { return; }
    u8* read = buf.start;
    u8* end = buf.cursor;
    while (read < end)
    {

    }
}

////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////
extern "C" ErrorCode Snd_ParseCommandString(const char* str, const char** tokens, const i32 numTokens)
{
    if (ZE_CompareStrings(tokens[0], "HELP") == 0)
    {
        printf("SND TEST - play test sound\n");
    }
    #if 1
    if (ZE_CompareStrings(tokens[0], "SND") == 0)
    {
        if (numTokens == 2 && ZE_CompareStrings(tokens[1],"TEST") == 0)
        {
            i32 index = 0;
            printf("SND - test play index %d\n", index);
            Snd_PlayQuick(index);
        }
        return 1;
    }
    #endif
    if (ZE_CompareStrings(tokens[0], "VERSION") == 0)
	{
		printf("SOUND Built %s: %s\n", __DATE__, __TIME__);
		return 0;
	}
    return 0;
}

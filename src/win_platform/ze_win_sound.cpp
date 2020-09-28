
//#include "../../lib/fmod/fmod_studio.hpp"
#include "../../lib/fmod/fmod.hpp"

#include "ze_win_sound.h"
#include "../ze_common/ze_common_full.h"

#define SND_MAX_SOUND_HANDLES 256
#define SND_MAX_CHANNELS 32

/////////////////////////////////////////////////////////////////////
// Internal data structures
/////////////////////////////////////////////////////////////////////
#if 0
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
#endif

struct ZSoundFmodEventType
{
    ZSoundEventType event;
    FMOD::Sound* handle;
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

API reading
https://documentation.help/FMOD-Studio-API/lowlevel_api_interfaces.html
https://documentation.help/FMOD-Studio-API/3dsound.html
https://documentation.help/FMOD-Studio-API/FMOD_Studio_EventInstance_Set3DAttributes.html
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

internal FMOD::System* sys = NULL;
internal FMOD::Sound* g_soundHandles[SND_MAX_SOUND_HANDLES];
internal i32 g_nextSoundHandle = 0;

internal ZSoundFmodEventType g_eventTypes[SND_MAX_SOUND_HANDLES];

internal f32 g_fxVolume = 0.0f;
internal f32 g_bgmVolume = 0.0f;

internal Transform g_listener;

//internal FMOD::Channel* g_channels[ND_MAX_CHANNELS];

// internal ZSound g_sounds[128];
// internal i32 g_nextSoundId = 0;
// internal ZSoundSource g_sources[128];

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

internal void Snd_Update3DListener(Transform* t)
{
    if (t != NULL)
    {
        g_listener = *t;
    }
    else
    {
        Transform_SetToIdentity(&g_listener);
    }
    
    FMOD_VECTOR vel = {};
    FMOD_RESULT result = sys->set3DListenerAttributes(
        0,
        (FMOD_VECTOR*)&g_listener.pos,
        &vel,
        (FMOD_VECTOR*)&g_listener.rotation.zAxis,
        (FMOD_VECTOR*)&g_listener.rotation.yAxis
        );
    if (result != FMOD_OK)
    {
        printf("SND error code %d updating listener %d\n", result, 0);
    }
}

#if 1 // Low level API only
extern "C" ErrorCode Snd_Init()
{
    printf("SOUND init\n");
    FMOD_RESULT result;
    //FMOD::System* system = NULL;

    result = FMOD::System_Create(&sys); // Create the Studio System object.
    if (result != FMOD_OK)
    {
        printf("Create FMOD system failed %d\n", result);
        return ZE_ERROR_UNKNOWN;
    }
    
    // Initialize FMOD Low Level
    result = sys->init(100, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        printf("Init fmod system failed %d\n", result);
        return ZE_ERROR_UNKNOWN;
    }

    Transform_SetToIdentity(&g_listener);
    #if 1
    result = sys->set3DNumListeners(1);
    if (result != FMOD_OK)
    {
        printf("SND error %d setting num 3d listeners\n", result);
        return ZE_ERROR_UNKNOWN;
    }
    Snd_Update3DListener(0);
    #endif
    sys->set3DSettings(10.f, 10.f, 10.f);

    //gsnd_channel->setVolume(g_fxVolume);

    printf("SND Initialised\n");
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
    FMOD::Sound* handle = *handlePtr;
    FMOD_MODE modeFlags = FMOD_3D | FMOD_LOOP_OFF | FMOD_3D_WORLDRELATIVE;
    FMOD_RESULT result = sys->createSound(filePath, modeFlags, NULL, handlePtr);
    
    handle->setLoopCount(0);
    handle->set3DMinMaxDistance(1.f, 5000.f);
    
    if (result == FMOD_OK)
    {
        printf("SND Loaded sound %s to index %d\n", filePath, index);
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
    ILLEGAL_CODE_PATH



    printf("SND Creating sound, %d bytes from %d\n", numBytes, (u32)data);
    FMOD_CREATESOUNDEXINFO info = {};
    info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    info.length = numBytes;

    i32 index;
    FMOD::Sound** handlePtr = Snd_GetFreeSoundHandle(&index);
    FMOD_RESULT result = sys->createSound((const char*)data, FMOD_OPENMEMORY, &info, handlePtr);
    // TODO: some samples seem to have an auto-loop flag.
    // want to disable here but doesn't seem to work:
    FMOD::Sound* handle = *handlePtr;
    handle->setLoopCount(0);

    printf(" create result: %d\n", result);
    if (result == FMOD_OK)
    {
        return index;
    }
    return -1;
}

extern "C" void Snd_SetupEvent(ZSoundEventType evType, char* soundName)
{
    ZSoundFmodEventType* ev = &g_eventTypes[evType.index];
    ev->event = evType;
    // TODO: attach samples
    //ev->handle =
}

////////////////////////////////////////////////////////
// Play
////////////////////////////////////////////////////////
extern "C" void Snd_PlayQuick(i32 sampleIndex, Vec3 pos)
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
    
    FMOD::Channel* chnl;
    FMOD_RESULT result = sys->playSound(handle, NULL, false, &chnl);
    chnl->setVolume(g_fxVolume);
    FMOD_MODE mode = 0;
    chnl->getMode(&mode);
    if ((mode & FMOD_3D) != 0)
    {
        result = chnl->set3DAttributes((FMOD_VECTOR*)&pos, NULL, NULL);
    }
    //chnl->setMode(FMOD_3D | FMOD_3D_WORLDRELATIVE);
    if (result != FMOD_OK)
    {
        printf("\tError %d playing sound %d\n", result, sampleIndex);
        return;
    }
    // Vec3 forward = g_listener.rotation.zAxis;
    // printf("Play %d at %.3f, %.3f, %.3f - listener at %.3f, %.3f, %.3f\n",
    //     sampleIndex,
    //     pos.x, pos.y, pos.z,
    //     g_listener.pos.x, g_listener.pos.y, g_listener.pos.z
    // );
}

extern "C" void Snd_ExecCommands(ZEBuffer* buf)
{
    ZSoundCommand* cmds = (ZSoundCommand*)buf->start;
    i32 numCommands = buf->Written() / sizeof(ZSoundCommand);

    for (i32 i = 0; i < numCommands; ++i)
    {
        ZSoundCommand* cmd = &cmds[i];
        //printf("SND - read event type %d\n", cmd->type);
        switch (cmd->type)
        {
            case ZSOUND_EVENT_PLAY:
            Snd_PlayQuick(cmd->data.play.soundEventType, cmd->data.play.pos);
            break;
            case ZSOUND_EVENT_SET_LISTENER:
            Snd_Update3DListener(&cmd->data.listener);
            break;
            default:
            printf("SND - unknown event type %d\n", cmd->type);
            break;
        }
        
    }
    sys->update();
}

internal f32 Snd_ParseVolume(const char* asci)
{
    // read as 0 to 100
    i32 levelInt = ZE_AsciToInt32(asci);
    // normalise
    f32 level = (f32)levelInt / 100.f;
    if (level < 0) { level = 0; }
    if (level > 1) { level = 1; }
    return level;
}

////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////
extern "C" ErrorCode Snd_ParseCommandString(const char* str, const char** tokens, const i32 numTokens)
{
    if (ZE_CompareStrings(tokens[0], "HELP") == 0)
    {
        printf("SND TEST <index> - play test sound\n");
        printf("SND FX <0 to 100> - set FX volume\n");
        printf("SND BGM <0 to 100> - set music volume\n");
        return 0;
    }
    if (ZE_CompareStrings(tokens[0], "VERSION") == 0)
	{
		printf("SOUND Built %s: %s\n", __DATE__, __TIME__);
		return 0;
	}

    if (ZE_CompareStrings(tokens[0], "SND") == 0)
    {
        if (numTokens == 3 && ZE_CompareStrings(tokens[1],"TEST") == 0)
        {
            i32 index = ZE_AsciToInt32(tokens[2]);
            printf("SND - test play index %d\n", index);
            f32 randX = COM_STDRandomInRange(-10, 10);
            //f32 randY = COM_STDRandomInRange(-50, 50);
            f32 randZ = COM_STDRandomInRange(-10, 10);
            Vec3 soundPos = { randX, 0, randZ };
            Snd_PlayQuick(index, soundPos);
            return 1;
        }
        if (numTokens == 3 && ZE_CompareStrings(tokens[1],"FX") == 0)
        {
            g_fxVolume = Snd_ParseVolume(tokens[2]);
            printf("SND set FX vol %.3f\n", g_fxVolume);
            return 1;
        }
        if (numTokens == 3 && ZE_CompareStrings(tokens[1], "BGM") == 0)
        {
            g_bgmVolume = Snd_ParseVolume(tokens[2]);
            printf("SND set BGM vol %.3f\n", g_bgmVolume);
            return 1;
        }
        printf("SND - unknown command or token mismatch\n");
        return 1;
    }
    return 0;
}

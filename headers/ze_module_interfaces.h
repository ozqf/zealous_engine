#ifndef ZE_MODULE_INTERFACES_H
#define ZE_MODULE_INTERFACES_H

#include "../headers/common/ze_common_full.h"
#include "../headers/zr_asset_db.h"

// timing information given to app each tick
struct app_frame_info
{
    i32 frameRate;
    i32 frameNumber;
    f32 interval;
    f32 time;
};

// app (game) module function pointers
struct ze_app_export
{
    i32     (*Init)();
    i32     (*Shutdown)();
    i32     (*Tick)(app_frame_info info);
    i32     (*WriteDraw)(void* zrViewFrame);
    i32     (*RendererReloaded)();
	// return YES if command was handled
    i32     (*ParseCommandString)(const char* str, const char** tokens, const i32 numTokens);

    i32     sentinel;
};

// window (renderer) module function pointers
struct ze_window_export
{
    i32 (*Init)();
    i32 (*MainLoop)();
    i32 (*IsMouseCaptured)();
    void (*SetMouseCaptured)(bool flag);
	Vec2 (*GetNormalisedMousePos)();
    // Written to by app, read by renderer
    void (*Acquire_AppDrawBuffers)(ZEBuffer** listBuf, ZEBuffer** dataBuf);
    void (*Release_AppDrawBuffers)();
    // For system events. written to by window, read by app
    void (*Acquire_EventBuffer)(ZEBuffer** buf);
    void (*Release_EventBuffer)();
    
    i32     (*ParseCommandString)(const char* str, const char** tokens, const i32 numTokens);
    i32 sentinel;
};

// platform module function pointers
struct ze_platform_export
{
    void (*Warning)(const char* msg);
    void (*Error)(const char* msg);
    void (*Log)(const char* msg);
    void (*Print)(const char* msg);
    void (*DebugBreak)();

    f64 (*QueryClock)();

    ZEFileIO files;
	ZEAllocator alloc;
	ZEIniFile* (*GetConfig)();

    // Shared asset manager
    ZRAssetDB* (*GetAssetDB)();
    void (*EnqueueTextCommand)(const char* str);
    void (*GetCmdLine)(i32* argc, char*** argv);

    // mouse state - position and buttons read via EventBuffer
    i32 (*IsMouseCaptured)();
    void (*SetMouseCaptured)(bool flag);
	Vec2 (*GetNormalisedMousePos)();
    
    // platform will pass to app for writing
    i32 (*AppWriteDraw)(void* zrViewFrame);

    // Acquire is passed through to window if it is available
    void (*Acquire_AppDrawBuffers)(ZEBuffer** listBuf, ZEBuffer** dataBuf);
    void (*Release_AppDrawBuffers)();
    void (*Acquire_EventBuffer)(ZEBuffer** buf);
    void (*Release_EventBuffer)();

    void (*OpenSocket)(i32* socketIndex, u16* port);
    void (*CloseSocket)(i32 socketIndex);
    i32 (*Send)(i32 socketIndex, ZNetAddress addr, u8* data, i32 dataSize);

    void (*LockMutex)(i32 index, i32 tag);
    void (*UnlockMutex)(i32 index, i32 tag);

    i32 (*GetVar)(i32 id);
    void (*SetVar)(i32 id, i32 value);

    // sound
    i32 (*SndLoadFile)(char* name, char* filePath);
    void (*SndPlayQuick)(i32 sampleIndex, Vec3 pos);
    void (*Snd_ExecCommands)(ZEBuffer* buf);


    i32 sentinel;
};

#define ZE_WINDOW_LINK_FUNC_NAME "ZE_LinkToWindowModule"
#define ZE_GAME_LINK_FUNC_NAME "ZE_LinkToGameModule"

#define ZE_DEFAULT_WINDOW_DLL_NAME "wingl.dll"
#define ZE_DEFAULT_APP_DLL_NAME "game.dll"
#define ZE_DEFAULT_APP_DIR "base"

#define ZE_MUTEX_DRAW_QUEUE 0
#define ZE_MUTEX_WINDOW_EVENTS 1
#define ZE_MUTEX_APP_TEXT_COMMAND_QUEUE 2
#define ZE_MUTEX_LAST__ 2

// Signature of platform <-> window linking function exported
// from window DLL
typedef ze_window_export (Func_LinkToWindow)(ze_platform_export kernelFuncs);

// Signature of platform <-> game linking function exported
// from game DLL
typedef ze_app_export (Func_LinkToApp)(ze_platform_export kernelFuncs);


#endif // ZE_MODULE_INTERFACES_H
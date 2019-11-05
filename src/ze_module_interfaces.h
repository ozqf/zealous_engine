#ifndef ZE_MODULE_INTERFACES_H
#define ZE_MODULE_INTERFACES_H

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_byte_buffer.h"

// app module function pointers
struct ze_app_export
{
    i32     isValid;
    i32     (*Init)();
    i32     (*Shutdown)();
    i32     (*Tick)();
    i32     (*RendererReloaded)();
    i32     sentinel;
    i32     (*ParseCommandString)(char* str, char** tokens, i32 numTokens);
};

// window module function pointers
struct ze_window_export
{
    i32 (*Init)();
    i32 (*MainLoop)();
    void (*Acquire_AppDrawBuffers)(ZEByteBuffer** listBuf, ZEByteBuffer** dataBuf);
    void (*Release_AppDrawBuffers)();
    void (*Acquire_EventBuffer)(ZEByteBuffer** buf);
    void (*Release_EventBuffer)();
    i32 sentinel;
};

// platform module function pointers
struct ze_platform_export
{
    void (*Warning)(char* msg);
    void (*Error)(char* msg);
    void (*Log)(char* msg);
    void (*Print)(char* msg);

    f64 (*QueryClock)();
    void* (*Allocate)(i32 numBytes);
    void (*Free)(void* ptr);
    
    // Acquire is passed through to window if it is availables
    void (*Acquire_AppDrawBuffers)(ZEByteBuffer** listBuf, ZEByteBuffer** dataBuf);
    void (*Release_AppDrawBuffers)();
    void (*Acquire_EventBuffer)(ZEByteBuffer** buf);
    void (*Release_EventBuffer)();

    void (*LockMutex)(i32 index, i32 tag);
    void (*UnlockMutex)(i32 index, i32 tag);
    i32 sentinel;
};

#define ZE_WINDOW_LINK_FUNC_NAME "ZE_LinkToWindowModule"
#define ZE_GAME_LINK_FUNC_NAME "ZE_LinkToGameModule"

#define ZE_DEFAULT_WINDOW_DLL_NAME "wingl.dll"
#define ZE_DEFAULT_APP_DLL_NAME "game.dll"
#define ZE_DEFAULT_APP_DIR "base"

#define ZE_MUTEX_DRAW_QUEUE 0
#define ZE_MUTEX_WINDOW_EVENTS 1

// Signature of platform <-> window linking function exported
// from window DLL
typedef ze_window_export (Func_LinkToWindow)(ze_platform_export kernelFuncs);

// Signature of platform <-> game linking function exported
// from game DLL
typedef ze_app_export (Func_LinkToApp)(ze_platform_export kernelFuncs);


#endif // ZE_MODULE_INTERFACES_H
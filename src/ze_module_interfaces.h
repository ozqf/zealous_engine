#ifndef ZE_MODULE_INTERFACES_H
#define ZE_MODULE_INTERFACES_H

#include "../ze_common/ze_common.h"

// app module function pointers
struct ze_app_export
{
    i32     isValid;
    i32     (*AppInit)();
    i32     (*AppShutdown)();
    i32     sentinel;
};

// window module function pointers
struct ze_window_export
{
    i32 (*Init)();
    i32 (*MainLoop)();
    i32 sentinel;
};

// platform module function pointers
struct ze_platform_export
{
    void (*Warning)(char* msg, char* title);
    void (*Error)(char* msg, char* title);

    double (*QueryClock)();
    void* (*Allocate)(i32 numBytes);
    void (*Free)(void* ptr);

    void (*LockMutex)(i32 index, i32 tag);
    void (*UnlockMutex)(i32 index, i32 tag);
    i32 sentinel;
};

#define ZE_WINDOW_LINK_FUNC_NAME "ZE_LinkToWindowModule"
#define ZE_GAME_LINK_FUNC_NAME "ZE_LinkToGameModule"

// Signature of platform <-> window linking function exported
// from window DLL
typedef ze_window_export (Func_LinkToWindow)(ze_platform_export kernelFuncs);

// Signature of platform <-> game linking function exported
// from game DLL
typedef ze_app_export (Func_LinkToApp)(ze_platform_export kernelFuncs);


#endif // ZE_MODULE_INTERFACES_H
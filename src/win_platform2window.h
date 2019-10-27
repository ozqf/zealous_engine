#ifndef ZE_WIN64_PLATFORM_2_RENDERER
#define ZE_WIN64_PLATFORM_2_RENDERER

#include "../ze_common/ze_common.h"

struct ze_window_export
{
    i32 (*Init)();
    i32 (*MainLoop)();
};

struct ze_kernel_export
{
    void (*Warning)(char* msg, char* title);
    void (*Error)(char* msg, char* title);

    double (*QueryClock)();
    void* (*Allocate)(i32 numBytes);
    void (*Free)(void* ptr);

    void (*LockMutex)(i32 index, i32 tag);
    void (*UnlockMutex)(i32 index, i32 tag);
};

#define ZE_WINDOW_LINK_FUNC_NAME "ZE_LinkToWindowModule"

// Signature of platform <-> window linking function exported
// from window DLL
typedef ze_window_export (Func_LinkToWindow)(ze_kernel_export kernelFuncs);

#endif // ZE_WIN64_PLATFORM_2_RENDERER
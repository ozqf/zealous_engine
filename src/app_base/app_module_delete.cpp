#ifndef APP_MODULE_CPP
#define APP_MODULE_CPP

#include "../ze_common/ze_common.h"

#include "../ze_module_interfaces.h"

static ze_platform_export g_platform = {};
static f64 g_lastTimeSample;

internal i32 AppImpl_Init()
{
    printf("App Init\n");
    g_lastTimeSample = g_platform.QueryClock();
    return ZE_ERROR_NONE;
}

internal i32 AppImpl_Shutdown()
{
    return ZE_ERROR_NONE;
}

internal i32 AppImpl_RendererReloaded()
{
    return ZE_ERROR_NONE;
}

internal i32 AppImpl_ParseCommandString(char* str, char** tokens, i32 numTokens)
{
    return NO;
}

internal i32 AppImpl_Tick()
{
    f64 time = g_platform.QueryClock();
    f64 diff = time - g_lastTimeSample;
    if (diff >= 1.0)
    {
        g_lastTimeSample = time;
        printf("App TOCK\n");
    }
    return ZE_ERROR_NONE;
}

extern "C"
ze_app_export __declspec(dllexport) ZE_LinkToGameModule(ze_platform_export platform)
{
    printf("APP linking to platform\n");
    g_platform = platform;
    ze_app_export appExport = {};
    appExport.Init = AppImpl_Init;
    appExport.Tick = AppImpl_Tick;
    appExport.ParseCommandString = AppImpl_ParseCommandString;
    appExport.RendererReloaded = AppImpl_RendererReloaded;
    appExport.Shutdown = AppImpl_Shutdown;
    appExport.sentinel = ZE_SENTINEL;
    return appExport;
}

#endif // APP_MODULE_CPP
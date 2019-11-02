#ifndef APP_MODULE_CPP
#define APP_MODULE_CPP

#include "../ze_common/ze_common.h"

#include "../ze_module_interfaces.h"

static ze_platform_export g_platform = {};

internal i32 AppImpl_Init()
{
    printf("App Init\n");
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

extern "C"
ze_app_export __declspec(dllexport) ZE_LinkToGameModule(ze_platform_export platform)
{
    printf("APP linking to platform\n");
    g_platform = platform;
    ze_app_export appExport = {};
    appExport.Init = AppImpl_Init;
    appExport.ParseCommandString = AppImpl_ParseCommandString;
    appExport.RendererReloaded = AppImpl_RendererReloaded;
    appExport.Shutdown = AppImpl_Shutdown;
    appExport.sentinel = ZE_SENTINEL;
    return appExport;
}

#endif // APP_MODULE_CPP
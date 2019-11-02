#ifndef APP_MODULE_CPP
#define APP_MODULE_CPP

#include "../ze_common/ze_common.h"
// export
#include "../app_interface.h"
// import
#include "../platform_interface.h"

static AppPlatform g_platform = {};

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
internal u8 AppImpl_ParseCommandString(char* str, char** tokens, i32 numTokens)
{
    return NO;
}

extern "C"
internal AppInterface ZE_LinkToApp(AppPlatform platform)
{
    g_platform = platform;
    AppInterface appExport = {};
    appExport.AppInit = AppImpl_Init;
    appExport.AppParseCommandString = AppImpl_ParseCommandString;
    appExport.AppRendererReloaded = AppImpl_RendererReloaded;
    appExport.AppShutdown = AppImpl_Shutdown;
    return appExport;
}

#endif // APP_MODULE_CPP
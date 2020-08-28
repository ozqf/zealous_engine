/*
App Stub - smallest possible app DLL implementation

*/
#include "../ze_common/ze_common.h"
#include "../zqf_renderer.h"
#include "../ze_module_interfaces.h"

internal ze_platform_export g_platform = {};

#define APP_FRAME_RATE 60
#define APP_FRAME_INTERVAL (1.f / (f32)APP_FRAME_RATE)

internal i32 g_frameCount = 0;
internal f64 g_lastTimeSample = 0;
internal i32 g_lastPlatformFrame = 0;

internal i32 App_TimeForFrame()
{
    timeFloat frameInterval = (timeFloat)APP_FRAME_INTERVAL;
    f64 time = g_platform.QueryClock();
    f64 diff = time - g_lastTimeSample;
    g_lastPlatformFrame++;
    if (diff > frameInterval)
    {
        g_lastTimeSample = time;
        g_frameCount++;
        return YES;
    }
    return NO;
}

internal void App_Frame(i32 frameNumber)
{
    // Acquire buffers
    ZEByteBuffer* list;
    ZEByteBuffer* data;
    g_platform.Acquire_AppDrawBuffers(&list, &data);
    list->Clear(NO);
    data->Clear(NO);

    // Prepare frame header    
    ZRViewFrame* frame = (ZRViewFrame*)list->cursor;
    list->cursor += sizeof(ZRViewFrame);
    *frame = {};
    frame->sentinel = ZR_SENTINEL;
    frame->frameNumber = frameNumber;
    frame->list = list;
    frame->data = data;

    // -- Add draw scenes here --

    // release
    g_platform.Release_AppDrawBuffers();
}

/***************************************
* Module export functions
***************************************/
internal i32  AppImpl_Init()
{
	printf("App stub Init\n");
	return ZE_ERROR_NONE;
}

internal i32  AppImpl_Shutdown()
{
    printf("App stub Shutdown\n");
    // Free memory, assuming a new APP might be loaded in it's place
    return ZE_ERROR_NONE;
}

internal i32 AppImpl_RendererReloaded()
{
	return ZE_ERROR_NONE;
}

internal i32 AppImpl_ParseCommandString(const char* str, const char** tokens, const i32 numTokens)
{
	return NO;
}

internal i32 AppImpl_Tick()
{
    if (App_TimeForFrame())
    {
        App_Frame(g_frameCount);
    }
    
    
	return ZE_ERROR_NONE;
}

/***************************************
* Export Windows DLL functions
***************************************/
#include <Windows.h>

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

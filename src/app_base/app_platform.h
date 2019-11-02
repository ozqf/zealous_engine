#pragma once

#include "stdlib.h"
#include "../sys_events.h"
#include "../ze_module_interfaces.h"

/***************************************
* Public (app.h)
***************************************/
extern "C"
void App_Log(char* msg)
{
    g_platform.Log(msg);
}

extern "C"
void App_Print(char* msg)
{
    g_platform.Print(msg);
}

extern "C"
void AppImpl_SetMouseCaptured()
{

}

extern "C"
void AppImpl_SetMouseFree()
{

}

extern "C"
void App_Error(char* msg)
{
    g_platform.Error(msg);
}

extern "C"
ScreenInfo App_GetScreenInfo()
{
	return g_screenInfo;
}

extern "C"
f32 App_GetSimFrameInterval()
{
    return (1.0f / g_simFrameRate);
}

extern "C"
i32 App_CalcTickInterval(f32 seconds)
{
    f32 result = seconds / App_GetSimFrameInterval();
    // round
    return (i32)(result + 0.5f);
}

extern "C"
f64 App_SampleClock()
{
    return g_platform.QueryClock();
}

extern "C"
void App_SetPerformanceTime(i32 index, i32 tick, f32 time)
{
    g_performanceStats[index].frames[tick % APP_MAX_PERFORMANCE_FRAMES] = time;
}

extern "C"
f32 App_GetPerformanceTime(i32 index)
{
    return g_performanceStats[index].Sum();
}

/***************************************
* Private
***************************************/

internal f32 App_CalcInterpolationTime(f32 accumulator, f32 interval)
{
    return (accumulator / interval);
}

internal void App_Fatal(const char* msg)
{
	g_platform.Error((char*)msg);
}

/***************************************
* Define functions accessible to platform
***************************************/
internal i32  g_isValid = 0;

internal i32  AppImpl_Init()
{
    APP_LOG(128, "App initialising. Build data %s - %s\n", __DATE__, __TIME__);
    //App_Log("Test Log\n");
	
    //App_Win32_AttachErrorHandlers();
	ZE_SetFatalError(App_Fatal);

    // Memory

    // Acquiring an old 'heap object here. Various platform functions
    // still use it for loading assets so can't remove. Future allocations
    // should just use the basic malloc tracker until further notice
    #if 0
    u32 heapMB = 64;
    u32 mainMemorySize = MegaBytes(heapMB);
    MemoryBlock mem = {};

    APP_LOG(128, "APP Requested %d MB for Heap\n", heapMB);

    if (!g_platform.Malloc(&mem, mainMemorySize))
    {
        APP_LOG(128, "APP Platform malloc failed\n");
		ZE_ASSERT(0, "APP Platform malloc failed");
        return 0;
    }
    else
    {
        Heap_Init(&g_heap, mem.ptrMemory, mem.size);
    }

    ///////////////////////////////////////////////////////////////
    // Assets
    ///////////////////////////////////////////////////////////////
    Tex_Init(&g_heap, g_platform);
    char * textures[] = 
    {
        "textures\\BitmapTest.bmp",
        DEFAULT_CONSOLE_CHARSET_PATH,
        "textures\\brbrick2.bmp",
        "textures\\W33_5.bmp",
        "textures\\COMP03_1.bmp",
        "\0"
    };
    //g_platform.Error("Bar", "Foo");
    APP_PRINT(64, "App load texture list\n");
    Tex_LoadTextureList(textures);

    COM_InitEmbeddedAssets();
    #endif
    // proc gen textures here
    //i32 texIndex = Tex_GetTextureIndexByName(sourceTextureName);
    //Texture2DHeader* h = &g_textureHandles.textureHeaders[texIndex];
    //Texture2DHeader* h = Tex_GetTextureByName(DEFAULT_CONSOLE_CHARSET_PATH);
    //Tex_GenerateBW(h);

    ///////////////////////////////////////////////////////////////

    // must be init AFTER textures as it needs teh console char sheet
    App_DebugInit();

    // Basic malloc tracker
    g_mallocs = COM_InitMallocList(g_mallocItems, APP_MAX_MALLOCS);

    i32 bufferSize = KiloBytes(64);

    // Internal loopbacks for client and server
    g_serverLoopback.a = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, bufferSize, "Server Loopback A"),
        bufferSize);
	g_serverLoopback.b = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, bufferSize, "Server Loopback B"),
        bufferSize);

    g_clientLoopback.a = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, bufferSize, "Client Loopback A"),
        bufferSize);
	g_clientLoopback.b = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, bufferSize, "Client Loopback B"),
        bufferSize);
    
	g_loopbackSocket.Init(g_fakeLagMinMS, g_fakeLagMaxMS, g_fakeLoss);
    
    g_localServerAddress = {};
    g_localServerAddress.port = APP_SERVER_LOOPBACK_PORT;
    #if 0
    // Render Scenes - orient camera
    RScene_Init(&g_worldScene, g_worldSceneItems, MAX_WORLD_SCENE_ITEMS,
		90, RENDER_PROJECTION_MODE_3D, 8);
    g_worldScene.cameraTransform.pos.z = 10;
    g_worldScene.cameraTransform.pos.y += 34;
    Transform_SetRotation(&g_worldScene.cameraTransform, -(80.0f    * DEG2RAD), 0, 0);
    #endif
    // server and client areas currently acquiring their own memory
    App_StartSession(APP_SESSION_TYPE_SINGLE_PLAYER);

    return ZE_ERROR_NONE;
}

internal i32  AppImpl_Shutdown()
{
    APP_LOG(128, "App Shutdown\n");
    
    // Free memory, assuming a new APP might be loaded in it's place
    /*
    MemoryBlock mem = {};
    mem.ptrMemory = g_heap.ptrMemory;
    mem.size = g_heap.size;
    g_platform.Free(&mem);
	*/
    //g_localClientSocket.Destroy();
    //g_localServerSocket.Destroy();

    return ZE_ERROR_NONE;
}

internal i32 App_EndSession()
{
    if (g_isRunningServer) { SV_Shutdown(); }
    if (g_isRunningClient) { CL_Shutdown(); }
    //ZNet_EndSession(g_serverNet);
    //ZNet_EndSession(g_clientNet);
    return ZE_ERROR_NONE;
}

internal i32 App_StartSession(i32 sessionType)
{
    APP_LOG(128, "\n=== START SESSION ===\n");
    switch (sessionType)
    {
        case APP_SESSION_TYPE_SINGLE_PLAYER:
        {
            App_EndSession();
            APP_LOG(128, "\tStarting single player\n");
            /*ZNet_StartSession(
                g_serverNet,
                NETMODE_DEDICATED_SERVER,
                NULL,
                APP_SERVER_LOOPBACK_PORT);*/
            //g_localServerSocket.isActive = 1;
            SV_Init();
            UserIds ids = SVU_CreateLocalUser();
            g_isRunningServer = 1;

            /*ZNet_StartSession(
                g_clientNet,
                NETMODE_CLIENT,
                &g_localServerAddress,
                APP_CLIENT_LOOPBACK_PORT);*/
            //g_localClientSocket.isActive = 1;
            ZNetAddress addr = {};
            addr.port = APP_SERVER_LOOPBACK_PORT;
            CL_Init(addr);
            CL_SetLocalUser(ids);
            g_isRunningClient = 1;

            return ZE_ERROR_NONE;
        } break;
        default:
        {
            APP_LOG(128, "Unknown Session type %d\n", sessionType);
            return ZE_ERROR_BAD_ARGUMENT;
        };
    }
    
}

//////////////////////////////////////////////////////////////
// Exported to platform
//////////////////////////////////////////////////////////////

internal void App_Input(i64 frameNumber, ZEByteBuffer commands)
{
    g_lastPlatformFrame = frameNumber;
	i32 inputBytes = commands.Written();
	//printf("APP input %d bytes\n", inputBytes);
	
    u8* read = commands.start;
    u8* end = commands.cursor;
    while (read < end)
    {
        SysEvent* header = (SysEvent*)read;
        ZE_ASSERT(
			Sys_ValidateEvent(header) == ZE_ERROR_NONE,
			"Error reading system event header")
        read += header->size;
		
        switch (header->type)
        {
            case SYS_EVENT_INPUT:
            {
                SysInputEvent* ev = (SysInputEvent*)header;
                //printf("APP input Event: %d value %d\n", ev->inputID, ev->value);
                ZEByteBuffer* b = g_clientLoopback.GetWrite();
                b->cursor += ZE_COPY(header, b->cursor, header->size);
            } break;
            case SYS_EVENT_PACKET:
            {
                // TODO: Load into loopbacks
            } break;

            case SYS_EVENT_SKIP: break;

            default:
            {
                APP_LOG(128, "APP Unknown sys event type %d size %d\n", header->type, header->size);
            } break;
        }
    }
}

i32 g_apptick = 0;
internal void App_SimFrame(f32 interval)
{
    AppTimer timer(APP_STAT_FRAME_TOTAL, g_apptick++);
    #if APP_DEBUG_LOG_FRAME_TIMING
        APP_LOG(64, "App Sim tick %.8f\n", interval);
    #endif
    App_UpdateLoopbackSocket(&g_loopbackSocket, interval);
    if (g_isRunningServer)
    {
        g_serverLoopback.Swap();
        g_serverLoopback.GetWrite()->Clear(NO);
        SV_Tick(g_serverLoopback.GetRead(), interval);
    }
    if (g_isRunningClient)
    {
        g_clientLoopback.Swap();
        g_clientLoopback.GetWrite()->Clear(NO);
        CL_Tick(g_clientLoopback.GetRead(), interval, g_lastPlatformFrame);
    }
}

internal void App_Update(f32 deltaTime)
{
    g_simFrameAcculator += deltaTime;
    f32 interval = App_GetSimFrameInterval();
    #if APP_DEBUG_LOG_FRAME_TIMING
        APP_LOG(128, "App Update DT: %.8f Accumulator: %.8f Interval: %.8f\n",
            time->deltaTime,
            g_simFrameAcculator,
            interval
            );
    #endif
    #if 1
    while (g_simFrameAcculator > interval)
    {
        APP_LOG(64, "FRAME\n");
        g_simFrameAcculator -= interval;
		App_SimFrame(interval);
    }
    #endif
    #if 0
    if (g_simFrameAcculator > interval)
    {
        g_simFrameAcculator -= interval;
		App_SimFrame(interval);
    }
    #endif
}

#if 0
// offset blocks of render objects left or right to show SV and CL side by side
internal void App_OffsetRenderObjects(RenderScene* scene, i32 firstItem, f32 x)
{
    for (u32 i = (u32)firstItem; i < scene->numObjects; ++i)
    {
        RenderListItem* item = &scene->sceneItems[i];
        item->transform.pos.x += x;
    }
}
#endif

internal i32 AppImpl_RendererReloaded()
{
    AppTimer timer(APP_STAT_RENDER_TOTAL, g_renderCalls++);

    char* texName = "textures\\white_bordered.bmp";
    //char* texName = "textures\\W33_5.bmp";
    //i32 texIndex = Tex_GetTextureIndexByName(texName);
    f32 interval = App_GetSimFrameInterval();
    f32 interpolationTime = App_CalcInterpolationTime(
        g_simFrameAcculator, interval);
    #if APP_DEBUG_LOG_FRAME_TIMING
        APP_LOG(128, "APP Interpolate: %.8f Accumulator %.8f interval: %.8f\n",
                interpolationTime,
                g_simFrameAcculator,
                interval
            );
    #endif
    #if 0 // New route
    RenderCommand* cmds;
    i32 numCommands;
    CL_GetRenderCommands(&cmds, &numCommands, texIndex, interpolationTime);

    g_platform.SubmitRenderCommands(cmds, numCommands);

    #endif

    #if 0 // Old route
    
    // Have to remember to do this or things explode:
    g_worldScene.numObjects = 0;

    SV_PopulateRenderScene(
        &g_worldScene, g_worldScene.maxObjects, texIndex, 1,
        g_debugRenderFlags & APP_REND_FLAG_SERVER_SCENE,
        g_debugRenderFlags & APP_REND_FLAG_SERVER_TESTS);
    
    CL_PopulateRenderScene(
        &g_worldScene.cameraTransform,
        &g_worldScene, g_worldScene.maxObjects, texIndex, interpolationTime);
    
    g_platform.RenderScene(&g_worldScene);

    App_WriteDebugStrings();
    
    g_platform.RenderScene(&g_debugScene);
    #endif
    return ZE_ERROR_NONE;
}

internal i32 AppImpl_ParseCommandString(char* str, char** tokens, i32 numTokens)
{
    if (numTokens == 2 && !ZE_CompareStrings(tokens[0], "DRAW"))
    {
        if (!ZE_CompareStrings(tokens[1], "SV"))
        { g_debugRenderFlags ^= APP_REND_FLAG_SERVER_SCENE; }

        return 1;
    }
    if (numTokens == 2 && !ZE_CompareStrings(tokens[0], "STAT"))
    {
        if (!ZE_CompareStrings(tokens[1], "CLS"))
        { g_debugPrintFlags = 0; }
        if (!ZE_CompareStrings(tokens[1], "APP"))
        { g_debugPrintFlags ^= APP_PRINT_FLAG_SPEEDS; }
        if (!ZE_CompareStrings(tokens[1], "SV"))
        { g_debugPrintFlags ^= APP_PRINT_FLAG_SERVER; }
        if (!ZE_CompareStrings(tokens[1], "CL"))
        { g_debugPrintFlags ^= APP_PRINT_FLAG_CLIENT; }
        return 1;
    }
    if (numTokens == 4 && !ZE_CompareStrings(tokens[0], "LAG"))
    {
        i32 minMS = ZE_AsciToInt32(tokens[1]);
        i32 maxMS = ZE_AsciToInt32(tokens[2]);
        f32 loss = (f32)ZE_AsciToInt32(tokens[3]) / 100.0f;
        g_loopbackSocket.SetLagStats(minMS, maxMS, loss);
        return 1;
    }
    if (g_isRunningServer && SV_ParseCommandString(str, tokens, numTokens)) { return 1; }
    if (g_isRunningClient && CL_ParseCommandString(str, tokens, numTokens)) { return 1; }
    return 0;
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

#if 0
extern "C"
AppInterface __declspec(dllexport) LinkToApp(AppPlatform platInterface)
{
    printf("APP: Library Built on %s at %s\n", __DATE__, __TIME__);
    g_platform = platInterface;
    g_isValid = true;

    AppInterface app;
    app.isValid = true;
    app.AppInit = AppImpl_Init;
    app.AppShutdown = App_Shutdown;
    app.AppRendererReloaded = App_RendererReloaded;
    //app.AppFixedUpdate = App_FixedFrame;
    //app.AppInput = App_Input;
    //app.AppUpdate = App_Update;
    //app.AppRender = App_Render;
    app.AppParseCommandString = AppImpl_ParseCommandString;
    return app;
}
#endif
extern "C"
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    // TODO: Find out why this called seamingly at random whilst running
    // ANSWER: For each thread that is started dllMain is called
    printf("APP DLL Main\n");
	return 1;
}

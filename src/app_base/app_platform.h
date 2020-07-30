#pragma once

#include "stdlib.h"
#include "../sys_events.h"
#include "../ze_module_interfaces.h"
#include "app_ui.h"
#include "app_internal.h"

/***************************************
* Public (app.h)
***************************************/
extern "C"
void App_Log(char* msg, i32 categoryMask)
{
    if (categoryMask & g_debugLogFlags)
    {
        g_platform.Log(msg);
    }
}

extern "C"
void App_Print(char* msg, i32 categoryMask)
{
    if (categoryMask & g_debugLogFlags)
    {
        g_platform.Print(msg);
    }
}

extern "C" i32 App_IsMouseCaptured()
{
    return g_platform.IsMouseCaptured();
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
ZRAssetDB* App_GetAssetDB()
{
    ZRAssetDB* db = (ZRAssetDB*)g_platform.GetAssetDB();
    // TODO: Handle this better.
    ZE_ASSERT(db != NULL, "Asset DB is null");
    return db;
}
#if 1
extern "C"
timeFloat App_GetSimFrameInterval()
{
    return (timeFloat)(1.0f / g_simFrameRate);
}
#endif
extern "C"
frameInt App_CalcTickInterval(timeFloat seconds)
{
    timeFloat result = seconds / App_GetSimFrameInterval();
    // round
    return (frameInt)(result + 0.5);
}

extern "C"
f64 App_SampleClock()
{
    return g_platform.QueryClock();
}

extern "C"
void App_SetPerformanceTime(i32 index, i32 tick, timeFloat time)
{
    g_performanceStats[index].frames[tick % APP_MAX_PERFORMANCE_FRAMES] = time;
}

extern "C"
timeFloat App_GetPerformanceTime(i32 index)
{
    return g_performanceStats[index].Sum();
}

extern "C" void App_Debug_GetServerSim(void** ptr)
{
    *ptr = NULL;
    //return SV_Debug_GetSimInstance(ptr);
}

extern "C" void App_DebugBreak()
{
    g_platform.DebugBreak();
}

/***************************************
* Private
***************************************/

internal timeFloat App_CalcInterpolationTime(timeFloat accumulator, timeFloat interval)
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
    
    // Open debug port
    g_debugPort = ZE_DEBUG_PORT;
    g_platform.OpenSocket(&g_debugSocket, &g_debugPort);
    printf("App - opened debug socket %d - port %d\n", g_debugSocket, g_debugPort);
	
    //App_Win32_AttachErrorHandlers();
	ZE_SetFatalError(App_Fatal);

    ZRAssetDB* assets = (ZRAssetDB*)g_platform.GetAssetDB();
    if (assets != NULL)
    {
        printf("APP - Got asset DB\n");
    }
    
    // must be init AFTER textures as it needs teh console char sheet
    App_DebugInit();

    // Basic malloc tracker
    g_mallocs = COM_InitMallocList(g_mallocItems, APP_MAX_MALLOCS);

    i32 bufferSize = KiloBytes(64);

    // Internal loopbacks for client and server
    g_gameBuffers.a = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, bufferSize, 0, "Game Buffer A"),
        bufferSize);
    g_gameBuffers.b = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, bufferSize, 0, "Game Buffer B"),
        bufferSize);
    
    g_soundBuffer = Buf_FromMalloc(
        COM_Malloc(&g_mallocs, bufferSize, 0, "Sound Events"),
        bufferSize);

	g_loopbackSocket.Init(g_fakeLagMinMS, g_fakeLagMaxMS, g_fakeLoss);
    
    g_localServerAddress = {};
    g_localServerAddress.port = APP_SERVER_LOOPBACK_PORT;
    
    // initialise sub-modules
    AppUI_Init();
    Game_Init();

    i32 index = -1;
    
    index = g_platform.SndLoadFile("weapon_pickup", "data/sound/Weapon_Pickup.wav");
    index = g_platform.SndLoadFile("weapon_switch", "data/sound/Weapon_Switch.wav");
    index = g_platform.SndLoadFile("heavy_shoot", "data/sound/EXPL_D_01.wav");
    index = g_platform.SndLoadFile("light_shoot", "data/sound/Boss_Shoot_Red_Oneshot.wav");
    index = g_platform.SndLoadFile("shield_break", "data/sound/Shield_Break.wav");

    // server and client areas currently acquiring their own memory
    App_StartSession(APP_SESSION_TYPE_SINGLE_PLAYER);

    return ZE_ERROR_NONE;
}

internal i32  AppImpl_Shutdown()
{
    APP_LOG(128, "App Shutdown\n");
    // Free memory, assuming a new APP might be loaded in it's place
    return ZE_ERROR_NONE;
}

internal i32 App_EndSession()
{
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
            
            // TODO: These will be indices to ports opened by the platform layer
            i32 serverPortId = -1;
            i32 clientPortId = -2;

            Game_Start();

            //////////////////////////////////////
            // Normal - auto create a local client
            #if 0
            // start server, add client immediately
            SV_Start();
            // creating a local user will start transmission to loopback buffer.
            // client must also be started!
            UserIds ids = SVU_CreateLocalUser();
            
            // client
            
            ZNetAddress addr = {};
            addr.port = APP_SERVER_LOOPBACK_PORT;
            CL_Start(addr, clientPortId);
            CL_SetLocalUser(ids);
            
            #endif
            
            
            //////////////////////////////////////
            // dev mode - local client will handshake
            #if 0

            // TODO: These will be indices to ports opened by the platform layer
            i32 serverPortId = -1;
            i32 clientPortId = -2;

            SV_Start();

            ZNetAddress addr = {};
            addr.port = APP_SERVER_LOOPBACK_PORT;
            CL_Start(addr, clientPortId);
            #endif

            
            /*ZNet_StartSession(
                g_serverNet,
                NETMODE_DEDICATED_SERVER,
                NULL,
                APP_SERVER_LOOPBACK_PORT);*/
            //g_localServerSocket.isActive = 1;

            /*ZNet_StartSession(
                g_clientNet,
                NETMODE_CLIENT,
                &g_localServerAddress,
                APP_CLIENT_LOOPBACK_PORT);*/
            //g_localClientSocket.isActive = 1;
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

internal void App_ReadSysEvents(ZEByteBuffer* events)
{
    u8* read = events->start;
    u8* end = events->cursor;
    i32 diff = end - read;
    if (diff == 0) { return; }
    
    ZEByteBuffer* gameInput = g_gameBuffers.GetWrite();
    while (read < end)
    {
        SysEvent* ev = (SysEvent*)read;
        read += ev->size;
        ErrorCode err = Sys_ValidateEvent(ev);
        if (err != ZE_ERROR_NONE)
        {
            ZE_ASSERT(NO, "Invalid system event");
        }
        if (ev->type == SYS_EVENT_PACKET)
        {
            // Is packet for client or server?
        }
        else if (ev->type == SYS_EVENT_INPUT)
        {
            BUF_COPY(gameInput, ev, ev->size);
        }
    }
}

internal void App_SimFrame(timeFloat interval)
{
    AppTimer timer(APP_STAT_FRAME_TOTAL, g_apptick++);
    #if APP_DEBUG_LOG_FRAME_TIMING
        APP_LOG(64, "App Sim tick %.8f\n", interval);
    #endif
    App_UpdateLoopbackSocket(&g_loopbackSocket, interval);
    ZEByteBuffer* events;
    g_platform.Acquire_EventBuffer(&events);
    App_ReadSysEvents(events);
    events->Clear(NO);
    g_platform.Release_EventBuffer();
    APP_LOG(64, "\nAPP Frame\n");

    g_gameBuffers.Swap();
    g_gameBuffers.GetWrite()->Clear(NO);
    Game_Tick(g_gameBuffers.GetRead(), &g_soundBuffer, interval);
    ZE_INIT_PTR_IN_PLACE(soundEv, ZSoundCommand, (&g_soundBuffer));
    soundEv->type = ZSOUND_EVENT_SET_LISTENER;
    soundEv->data.listener = Game_GetCamera();
}

internal i32 AppImpl_RendererReloaded()
{
    AppTimer timer(APP_STAT_RENDER_TOTAL, g_renderCalls++);
	timeFloat interval = App_GetSimFrameInterval();
    timeFloat interpolationTime = App_CalcInterpolationTime(
        g_simFrameAcculator, interval);
    #if APP_DEBUG_LOG_FRAME_TIMING
        APP_LOG(128, "APP Interpolate: %.8f Accumulator %.8f interval: %.8f\n",
                interpolationTime,
                g_simFrameAcculator,
                interval
            );
    #endif
    return ZE_ERROR_NONE;
}

internal i32 AppImpl_ParseCommandString(const char* str, const char** tokens, const i32 numTokens)
{
    // fall through
    if (ZE_CompareStrings(tokens[0], "VERSION") == 0)
	{
		printf("APP Built %s: %s\n", __DATE__, __TIME__);
	}
	if (!ZE_CompareStrings(tokens[0], "HELP"))
    {
        printf("DRAW SV - toggle drawing local server ontop of client scene\n");
        printf("STAT <CLS, APP, SV or CL> - toggle disabling debug text\n");
        printf("LAG <minMS, maxMS, loss> - set fake lag conditions\n");
        printf("\t eg LAG 100 200 5 for 100-200 ms lag and 5%% packet loss\n");
    }
    if (numTokens == 2 && !ZE_CompareStrings(tokens[0], "DRAW"))
    {
        if (!ZE_CompareStrings(tokens[1], "SV"))
        { g_debugRenderFlags ^= APP_REND_FLAG_SERVER_SCENE; }

        return YES;
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
        return YES;
    }
    if (numTokens == 4 && !ZE_CompareStrings(tokens[0], "LAG"))
    {
        i32 minMS = ZE_AsciToInt32(tokens[1]);
        i32 maxMS = ZE_AsciToInt32(tokens[2]);
        f32 loss = (f32)ZE_AsciToInt32(tokens[3]) / 100.0f;
        g_loopbackSocket.SetLagStats(minMS, maxMS, loss);
        return YES;
    }
    return NO;
}

internal void App_DrawFrame()
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
    frame->frameNumber = g_framesWritten++;
    frame->list = list;
    frame->data = data;

    // Add draw data
    Game_WriteDrawFrame(frame);
	AppUI_WriteFrame(frame);

    // release
    g_platform.Release_AppDrawBuffers();
}

internal i32 AppImpl_Tick()
{
    timeFloat frameInterval = App_GetSimFrameInterval();
    f64 time = g_platform.QueryClock();
    f64 diff = time - g_lastTimeSample;
    g_lastPlatformFrame++;
    if (diff >= frameInterval)
    {
        g_lastTimeSample = time;
        App_SimFrame(frameInterval);
        App_TickDebugUtils(frameInterval);
        App_DrawFrame();
        g_platform.Snd_ExecCommands(&g_soundBuffer);
        g_soundBuffer.Clear(NO);
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

extern "C"
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    // TODO: Find out why this called seamingly at random whilst running
    // ANSWER: For each thread that is started dllMain is called
    //printf("APP DLL Main\n");
	return 1;
}

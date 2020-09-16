#ifndef WIN64_ZE_KERNEL_CPP
#define WIN64_ZE_KERNEL_CPP
/*
Zealous Engine Executable.
Hosts:
	Window DLL (renderer and window control/events)
	App DLL (game logic)
	Sound DLL (audio, surprisingly)
Provides:
	I/O (file system, networking)
	Inter-DLL communication
	Hot reloading of DLLs
*/
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <stdio.h>

#define FORCE_OPTIMUS
#ifdef FORCE_OPTIMUS
// This export should force nvidia optimus drivers to use the dedicated video card
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
extern "C"
{
  __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
// AMD version
// http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
extern "C"
{
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif // FORCE_OPTIMUS

#include "../ze_module_interfaces.h"
#include "../ze_common/ze_common.h"
#include "../ze_common/ze_byte_buffer.h"
#include "../assetdb/zr_asset_db.h"

#include "ze_win_socket.h"
#include "ze_win_sound.h"

////////////////////////////////////////////////////////
// Data types
////////////////////////////////////////////////////////
struct ze_thread
{
    i32 sentinel;
    char* label;
};

struct ze_windows_thread
{
    ze_thread header;
    // Windows specific
    DWORD threadId;
    HANDLE handle;
};

static ze_platform_export Win_BuildExport();

////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////

// store current app path
#define MAX_APP_FOLDER_LEN 64
static char g_appFolderBuf[MAX_APP_FOLDER_LEN];

static ze_windows_thread g_appThread = {};
static volatile i32 g_bExitAppThread = NO;

static HMODULE g_windowDLL;
static ze_window_export g_window;

static HMODULE g_appDLL;
static ze_app_export g_app;
static i32 g_appFrameRate = 60;
static f64 g_lastAppTime = 0;
static i32 g_appFrameNumber = 0;

static FILE* g_logFile = NULL;

global_variable HWND consoleHandle;

#define MAX_MUTEXES 2
internal HANDLE g_mutexes[MAX_MUTEXES];

// performance counts per second
// Read Quad Part for a complete 64 bit integer
static LARGE_INTEGER g_timerFrequency;
// time since application startup. NEVER change once set.
static i64 g_clockBase;

#define ZE_TEST_TRANSMIT_PORT 55678
#define ZE_TEST_DEST_PORT 55679

static u16 g_diagnosticTransPort = ZE_TEST_TRANSMIT_PORT;
static u16 g_diagnosticDestPort = ZE_TEST_DEST_PORT;
static i32 g_diagnosticSocket;

// Asset db is held by the platform and renderer must attach
// uploader to it. DB must exist for all time that modules are loaded.
static ZRAssetDB* g_assets = NULL;

////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////
static void Win_Error(char *msg)
{
	printf("FATAL: %s\n", msg);
    // TODO: LPCWSTR means unicode, but treated as ascii!
    MessageBox(0, (LPCSTR)msg, (LPCSTR)"Error", MB_OK | MB_ICONINFORMATION);
	DebugBreak();
}

static void Win_Warning(char *msg)
{
	printf("WARNING: %s\n", msg);
    // TODO: LPCWSTR means unicode, but treated as ascii!
    MessageBox(0, (LPCSTR)msg, (LPCSTR)"Warning", MB_OK | MB_ICONINFORMATION);
}

static void Win_Log(char *msg)
{
	if (g_logFile != NULL)
    {
        fprintf(g_logFile, "%s", msg);
    }
}

static void Win_Print(char *msg)
{
	printf(msg);
}

////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////
static void Win_InitTimer()
{
	// Counts per second of performance frequency
	// eg 2742188
    QueryPerformanceFrequency(&g_timerFrequency);
	printf("Precision timer freq %lld\n", g_timerFrequency.QuadPart);
	LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
	g_clockBase = counter.QuadPart;
}

static void Win_InitMutexes()
{
    g_mutexes[ZE_MUTEX_DRAW_QUEUE] = CreateMutexA(0, 0, "DrawQueue");
    g_mutexes[ZE_MUTEX_WINDOW_EVENTS] = CreateMutexA(0, 0, "WindowEventQueue");
}

////////////////////////////////////////////////////////
// DLL Linking
////////////////////////////////////////////////////////

static ErrorCode LinkToGameDLL(char* directoryName, char* dllName)
{
    char str[128];
    i32 strLen = sprintf_s(str, 128, "%s\\%s", directoryName, dllName);
    printf("Loading App DLL \"%s\"\n", str);
    g_appDLL = LoadLibraryA(str);
    if (g_appDLL == NULL)
    {
        return ZE_ERROR_LINK_UP_FAILED;
    }
    Func_LinkToApp* linkPtr = (Func_LinkToApp*)GetProcAddress(g_appDLL, ZE_GAME_LINK_FUNC_NAME);
    if (linkPtr == NULL)
    {
        return ZE_ERROR_LINK_UP_FAILED;
    }
    ze_platform_export kernel_export = Win_BuildExport();
    g_app = {};

    g_app = linkPtr(kernel_export);
    if (g_app.sentinel != ZE_SENTINEL)
    {
        return ZE_ERROR_LINK_UP_FAILED;
    }
    ErrorCode err = g_app.Init();
    if (err != ZE_ERROR_NONE)
    {
        return ZE_ERROR_LINK_UP_FAILED;
    }
    return ZE_ERROR_NONE;
}

static ErrorCode LinkToWindowDLL(char* dllName)
{
    printf("Loading window DLL \"%s\"\n", dllName);
    g_windowDLL = LoadLibraryA(dllName);
    if (g_windowDLL == NULL)
    {
        return 1;
    }
    Func_LinkToWindow* linkPtr = (Func_LinkToWindow*)GetProcAddress(g_windowDLL, ZE_WINDOW_LINK_FUNC_NAME);
    if (linkPtr == NULL)
    {
        return 1;
    }
    ze_platform_export kernelExport = Win_BuildExport();
    g_window = {};

	g_window = linkPtr(kernelExport);
    if (g_window.Init == NULL)
    {
        return 1;
    }
    ErrorCode err = g_window.Init();
    if (err != ZE_ERROR_NONE)
    {
        return 1;
    }

    return ZE_ERROR_NONE;
}

////////////////////////////////////////////////////////
// Exported function implementations
////////////////////////////////////////////////////////

static i32 PlatformImpl_IsMouseCaptured()
{
    if (g_window.sentinel == ZR_SENTINEL)
    {
        return g_window.IsMouseCaptured();
    }
    return NO;
}

static void PlatformImpl_DebugBreak()
{
    DebugBreak();
}

static void* PlatformImpl_GetAssetDB()
{
    return g_assets;
}

static f64 PlatformImpl_QueryClock()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
	i64 ticksSinceStart = counter.QuadPart - g_clockBase;
	f64 secondsSinceStart =
		(f64)((f64)ticksSinceStart / (f64)g_timerFrequency.QuadPart);
	//printf("%f seconds since startup\n", secondsSinceStart);
    return secondsSinceStart;
}

static void PlatformImpl_LockMutex(i32 index, i32 tag)
{
    ZE_ASSERT(index >= 0 && index < MAX_MUTEXES,
        "out of bounds mutex index\n");
    DWORD result = WaitForSingleObject(g_mutexes[index], INFINITE);
	if (result != WAIT_OBJECT_0)
	{
		DWORD err = GetLastError();
		printf("Lock result is 0x%X err 0x%X\n", result, err);
		ILLEGAL_CODE_PATH
	}
}

static void PlatformImpl_UnlockMutex(i32 index, i32 tag)
{
    ZE_ASSERT(index >= 0 && index < MAX_MUTEXES, "out of bounds mutex index\n");
    DWORD result = ReleaseMutex(g_mutexes[index]);
	if (result == 0)
	{
		DWORD err = GetLastError();
		printf("Release result is 0x%X err 0x%X\n", result, err);
		ILLEGAL_CODE_PATH
	}
}

static void* PlatformImpl_Allocate(i32 numBytes)
{
    // TODO: Track allocations!
    return malloc(numBytes);
}

static void PlatformImpl_Free(void* ptr)
{
    free(ptr);
}

static i32 PlatformImpl_AppWriteDraw(void* zrViewFrame)
{
    return g_app.WriteDraw(zrViewFrame);
}

static void PlatformImpl_GetAppDrawbuffers(ZEByteBuffer** listBuf, ZEByteBuffer** dataBuf)
{
    if (g_window.sentinel == ZE_SENTINEL)
    {
        g_window.Acquire_AppDrawBuffers(listBuf, dataBuf);
    }
}

static void PlatformImpl_ReleaseAppDrawBuffers()
{
    if (g_window.sentinel == ZE_SENTINEL)
    {
        g_window.Release_AppDrawBuffers();
    }
}

static void PlatformImpl_AcquireEventBuffer(ZEByteBuffer** buf)
{
    if (g_window.sentinel == ZE_SENTINEL)
    { g_window.Acquire_EventBuffer(buf); }
    else { *buf = NULL; }
}

static void PlatformImpl_ReleaseEventBuffer()
{
    if (g_window.sentinel == ZE_SENTINEL)
    { g_window.Release_EventBuffer(); }
}

static i32 PlatformImpl_ExecTextCommand(
    const char* str, const i32 len, const char** tokens, const i32 numTokens)
{
    // call all loaded modules to attempt to run the given command
    // a result of true from any module means the command was
    // handled
    if (ZE_CompareStrings(str, "MANIFEST") == 0)
	{
		ZRDB_PrintManifest(g_assets);
		return YES;
	}
    if (Snd_ParseCommandString(str, tokens, numTokens))
    {
        return YES;
    }

    // fall through
    if (ZE_CompareStrings(tokens[0], "VERSION") == 0)
	{
		printf("PLATFORM Built %s: %s\n", __DATE__, __TIME__);
	}
    if (ZE_CompareStrings(str, "HELP") == 0)
    {
        printf("MANIFEST - list assets loaded to heap\n");
    }
	return (g_app.ParseCommandString(str, tokens, numTokens) == YES);
}

static void PlatformImpl_OpenSocket(i32* socket, u16* port)
{
    *socket = Net_OpenSocket(*port, port);
}

static void PlatformImpl_CloseSocket(i32 index)
{
    // TODO error check response and handle it... somehow
    i32 err = Net_CloseSocket(index);
}

static i32 PlatformImpl_Send(i32 socketIndex, ZNetAddress addr, u8* data, i32 dataSize)
{
    /*printf("ZWin - send %dB from socket %d to %d.%d.%d.%d:%d\n",
        dataSize,
        socketIndex,
        addr.ip4Bytes[0],
        addr.ip4Bytes[1],
        addr.ip4Bytes[2],
        addr.ip4Bytes[3],
        addr.port
    );*/
    i32 result = Net_SendTo(socketIndex, &addr, data, dataSize);
    return result;
}

static ze_platform_export Win_BuildExport()
{
    ze_platform_export result = {};
    result.Warning = Win_Warning;
    result.Error = Win_Error;
    result.Log = Win_Log;
    result.Print = Win_Print;
    result.DebugBreak = PlatformImpl_DebugBreak;

    result.GetAssetDB = PlatformImpl_GetAssetDB;
    result.ExecTextCommand = PlatformImpl_ExecTextCommand;
    result.IsMouseCaptured = PlatformImpl_IsMouseCaptured;

    result.QueryClock = PlatformImpl_QueryClock;
    result.Allocate = PlatformImpl_Allocate;
    result.Free = PlatformImpl_Free;

    result.LockMutex = PlatformImpl_LockMutex;
    result.UnlockMutex = PlatformImpl_UnlockMutex;

    result.AppWriteDraw = PlatformImpl_AppWriteDraw;
    result.Acquire_AppDrawBuffers = PlatformImpl_GetAppDrawbuffers;
    result.Release_AppDrawBuffers = PlatformImpl_ReleaseAppDrawBuffers;
    result.Acquire_EventBuffer = PlatformImpl_AcquireEventBuffer;
    result.Release_EventBuffer = PlatformImpl_ReleaseEventBuffer;

    result.OpenSocket = PlatformImpl_OpenSocket;
    result.CloseSocket = PlatformImpl_CloseSocket;
    result.Send = PlatformImpl_Send;

    result.SndLoadFile = Snd_LoadSoundWavFile;
    result.SndPlayQuick = Snd_PlayQuick;
    result.Snd_ExecCommands = Snd_ExecCommands;

    result.sentinel = ZE_SENTINEL;

    return result;
}

////////////////////////////////////////////////////////
// App Thread
////////////////////////////////////////////////////////
static i32 CheckTimeForAppFrame(app_frame_info* info)
{
    f32 interval = 1.f / (f32)g_appFrameRate;
    timeFloat frameInterval = (timeFloat)interval;
    f64 time = PlatformImpl_QueryClock();
    f64 diff = time - g_lastAppTime;
    if (diff > frameInterval)
    {
        g_lastAppTime = time;
        g_appFrameNumber++;

        info->frameRate = g_appFrameRate;
        info->frameNumber = g_appFrameNumber;
        info->interval = interval;
        info->time = (f32)time;
        return YES;
    }
    return NO;
}

static DWORD __stdcall AppThreadStartup(LPVOID lpThreadParameter)
{
    printf("Start App Thread\n");
    ErrorCode err = ZE_ERROR_NONE;

    while (!g_bExitAppThread)
    {
        app_frame_info info;
        if (CheckTimeForAppFrame(&info))
        {
            err = g_app.Tick(info);
            if (err != ZE_ERROR_NONE)
            {
                Win_Error("Error during app tick");
                g_app.Shutdown();
                g_bExitAppThread = YES;
            }
        }
    }

    #if 0 // timing test
	f64 startTime = PlatformImpl_QueryClock();
	f64 time = startTime;
	f64 lastTick = time;
    while(!g_bExitAppThread)
    {
		time = PlatformImpl_QueryClock();
		f64 diff = time - lastTick;
		//printf("Time diff %f\n", diff);
		if (diff >= 1.0)
		{
			lastTick = time;
			printf("App tick\n");
		}
    }
    #endif 
    printf("Exit App Thread\n");
    return 0;
}

static ErrorCode AppThread_Init()
{
    // link to app dll
    //char* appFolder = ZE_DEFAULT_APP_DIR;
    //char* appFolder = "stub";
    // i32 len = ZE_StrLen(appFolder);
    // if (len > MAX_APP_FOLDER_LEN)
    // {
    //     Win_Error("App folder name is too long\n");
    //     return ZE_ERROR_BAD_ARGUMENT;
    // }
    
    ErrorCode err = LinkToGameDLL(g_appFolderBuf, ZE_DEFAULT_APP_DLL_NAME);
    if (err != ZE_ERROR_NONE)
    {
        Win_Error("Error linking to App DLL");
        return err;
    }

    // Start thread
    g_appThread = {};
    g_appThread.header.sentinel = ZE_SENTINEL;
    g_appThread.header.label = "App Thread";
    g_appThread.handle = CreateThread(
        0, 0, AppThreadStartup, &g_appThread, 0, &g_appThread.threadId);
    return ZE_ERROR_NONE;
}

/**
 * Important aspect here: The platform is not in charge of the main loop.
 * The window does
 */
static void MainLoop()
{
	g_window.MainLoop();
}

////////////////////////////////////////////////////////
// Entry point
////////////////////////////////////////////////////////
int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    // init live debug console
    FILE *stream;
    AllocConsole();
    freopen_s(&stream, "conin$", "r", stdin);
    freopen_s(&stream, "conout$", "w", stdout);
    freopen_s(&stream, "conout$", "w", stderr);
    consoleHandle = GetConsoleWindow();
    MoveWindow(consoleHandle, 1, 1, 680, 600, 1);
    printf("[%s] Console initialized.\n", __FILE__);

    //////////////////////////////////////////////////////
    // Init log file

    // Build file name
    SYSTEMTIME time;
    GetSystemTime(&time);
    ZE_BUILD_STRING(logFileName, 128, "ex90_log_%d_%d_%d - %d_%d_%d.txt",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond
    );

    // Open log file
    #if 0
    errno_t fileErr = fopen_s(&g_logFile, logFileName, "w");
    if (fileErr != 0)
    {
        printf("Failed to open log file %s for writing\n", logFileName);
    }
	#endif

    //////////////////////////////////////////////////////
    char* tokens[32];
    char buf[512];
    i32 numTokens = ZE_ReadTokens(lpCmdLine, buf, tokens, 32);
    printf("%d cmd line tokens\n", numTokens);
    for (i32 i = 0; i < numTokens; ++i)
    {
        printf("> %s\n", tokens[i]);
    }
    i32 baseDirToken = ZE_FindParamIndex(tokens, numTokens, "-g");
    if (baseDirToken >= 0 && baseDirToken < (numTokens -1))
    {
        char* dir = tokens[baseDirToken + 1];
        printf("Base dir: %s\n", dir);
        ZE_CopyStringLimited(dir, g_appFolderBuf, MAX_APP_FOLDER_LEN);
    }
    else
    {
        ZE_CopyStringLimited(ZE_DEFAULT_APP_DIR, g_appFolderBuf, MAX_APP_FOLDER_LEN);
    }

	Win_InitTimer();

    // Create Mutexes
    Win_InitMutexes();

    // init network incase it is needed
    Net_Init();

    // alloc asset db
    g_assets = ZRDB_Create();

    // init sound
    i32 sndErr = Snd_Init();
    if (sndErr != ZE_ERROR_NONE)
    {
        Win_Error("Error starting sound");
        return 1;
    }

    #if 0
    g_diagnosticSocket = Net_OpenSocket(ZE_TEST_TRANSMIT_PORT, &g_diagnosticTransPort);
    printf("Winsock initialised - diagnostic socket %d on port %d\n",
        g_diagnosticSocket, g_diagnosticTransPort);
    char msg[128];
    i32 msgLen = sprintf_s(msg, 128, "Hello, World!\n");
    ZNetAddress addr = ZE_LocalHost(ZE_TEST_DEST_PORT);
    i32 netError = Net_SendTo(g_diagnosticSocket, &addr, (u8*)msg, msgLen);
    printf("Sent test udp packet - error code %d\n", netError);
    #endif
    ErrorCode err;

    // Attach to window
    err = LinkToWindowDLL(ZE_DEFAULT_WINDOW_DLL_NAME);
    if (err != ZE_ERROR_NONE)
    {
        Win_Error("Error connecting to window DLL");
        return 1;
    }

    // Window is okay. Begin App thread
    AppThread_Init();

    // window/render thread loop
    MainLoop();
    
    //Win32_Warning("window module says 666", "Okay");

    return 0;
}

#endif // WIN64_ZE_KERNEL_CPP
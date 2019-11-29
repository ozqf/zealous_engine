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
#include "../ze_module_interfaces.h"

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_byte_buffer.h"

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
static ze_windows_thread g_appThread = {};
static volatile i32 g_bExitAppThread = NO;

static HMODULE g_windowDLL;
static ze_window_export g_window;

static HMODULE g_appDLL;
static ze_app_export g_app;

static FILE* g_logFile = NULL;

global_variable HWND consoleHandle;

#define MAX_MUTEXES 2
internal HANDLE g_mutexes[MAX_MUTEXES];

// performance counts per second
// Read Quad Part for a complete 64 bit integer
static LARGE_INTEGER g_timerFrequency;
// time since application startup. NEVER change once set.
static i64 g_clockBase;

////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////
static void Win_Error(char *msg)
{
	printf("FATAL: %s\n", msg);
    MessageBox(0, msg, "Error", MB_OK | MB_ICONINFORMATION);
	DebugBreak();
}

static void Win_Warning(char *msg)
{
	printf("WARNING: %s\n", msg);
    MessageBox(0, msg, "Warning", MB_OK | MB_ICONINFORMATION);
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

static void PlatformImpl_DebugBreak()
{
    DebugBreak();
}

static void* PlatformImpl_GetAssetDB()
{
    if (g_window.sentinel == ZE_SENTINEL)
    {
        return g_window.GetAssetDB();
    }
    return NULL;
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

static ze_platform_export Win_BuildExport()
{
    ze_platform_export result = {};
    result.Warning = Win_Warning;
    result.Error = Win_Error;
    result.Log = Win_Log;
    result.Print = Win_Print;
    result.DebugBreak = PlatformImpl_DebugBreak;
    result.GetAssetDB = PlatformImpl_GetAssetDB;

    result.QueryClock = PlatformImpl_QueryClock;
    result.Allocate = PlatformImpl_Allocate;
    result.Free = PlatformImpl_Free;

    result.LockMutex = PlatformImpl_LockMutex;
    result.UnlockMutex = PlatformImpl_UnlockMutex;

    result.Acquire_AppDrawBuffers = PlatformImpl_GetAppDrawbuffers;
    result.Release_AppDrawBuffers = PlatformImpl_ReleaseAppDrawBuffers;
    result.Acquire_EventBuffer = PlatformImpl_AcquireEventBuffer;
    result.Release_EventBuffer = PlatformImpl_ReleaseEventBuffer;

    return result;
}

////////////////////////////////////////////////////////
// App Thread
////////////////////////////////////////////////////////
static DWORD __stdcall AppThreadStartup(LPVOID lpThreadParameter)
{
    printf("Start App Thread\n");
    
    ErrorCode err = LinkToGameDLL(ZE_DEFAULT_APP_DIR, ZE_DEFAULT_APP_DLL_NAME);
    if (err != ZE_ERROR_NONE)
    {
        Win_Error("Error linking to App DLL");
        return 1;
    }

    while (!g_bExitAppThread)
    {
        err = g_app.Tick();
        if (err != ZE_ERROR_NONE)
        {
            Win_Error("Error during app tick");
            g_app.Shutdown();
            g_bExitAppThread = YES;
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

static void AppThread_Init()
{
    g_appThread = {};
    g_appThread.header.sentinel = ZE_SENTINEL;
    g_appThread.header.label = "App Thread";
    g_appThread.handle = CreateThread(
        0, 0, AppThreadStartup, &g_appThread, 0, &g_appThread.threadId);
}

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
    
    // open
    errno_t fileErr = fopen_s(&g_logFile, logFileName, "w");
    if (fileErr != 0)
    {
        printf("Failed to open log file %s for writing\n", logFileName);
    }
	
	Win_InitTimer();

    // Create Mutexes
    Win_InitMutexes();

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
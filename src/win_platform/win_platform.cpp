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
#include "../win_platform2window.h"
#include "../ze_platform.h"

#include "../ze_common/ze_common.h"

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

////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////
static ze_windows_thread g_appThread = {};
static volatile i32 g_bExitAppThread = NO;

static HMODULE g_windowDLL;
static ze_window_export g_window;
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
static void Win_Error(char *msg, char *title)
{
	printf("FATAL: %s: %s\n", title, msg);
    MessageBox(0, msg, title, MB_OK | MB_ICONINFORMATION);
	DebugBreak();
}

static void Win_Warning(char *msg, char *title)
{
	printf("FATAL: %s: %s\n", title, msg);
    MessageBox(0, msg, title, MB_OK | MB_ICONINFORMATION);
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
// Exported function implementations
////////////////////////////////////////////////////////

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

static ze_kernel_export Win_BuildExport()
{
    ze_kernel_export result = {};
    result.Warning = Win_Warning;
    result.Error = Win_Error;
    result.QueryClock = PlatformImpl_QueryClock;
    result.Allocate = PlatformImpl_Allocate;
    result.Free = PlatformImpl_Free;
    result.LockMutex = PlatformImpl_LockMutex;
    result.UnlockMutex = PlatformImpl_UnlockMutex;
    return result;
}

////////////////////////////////////////////////////////
// App Thread
////////////////////////////////////////////////////////
static DWORD __stdcall AppThreadStartup(LPVOID lpThreadParameter)
{
    printf("Start App Thread\n");
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
    FILE *stream;
    AllocConsole();
    freopen_s(&stream, "conin$", "r", stdin);
    freopen_s(&stream, "conout$", "w", stdout);
    freopen_s(&stream, "conout$", "w", stderr);
    consoleHandle = GetConsoleWindow();
    MoveWindow(consoleHandle, 1, 1, 680, 600, 1);
    printf("[%s] Console initialized.\n", __FILE__);
	
	Win_InitTimer();

    // Create Mutexes
    Win_InitMutexes();

    // Attach to window
    printf("Loading window DLL \"%s\"\n", ZE_DEFAULT_WINDOW_DLL_NAME);
    HMODULE dll = LoadLibraryA(ZE_DEFAULT_WINDOW_DLL_NAME);
    if (dll == NULL)
    {
        Win_Warning("Failed to locate dll", "Warning");
        return 1;
    }
    Func_LinkToWindow* linkPtr = (Func_LinkToWindow*)GetProcAddress(dll, ZE_WINDOW_LINK_FUNC_NAME);
    if (linkPtr == NULL)
    {
        Win_Warning("Failed to find linking function", "Warning");
        return 1;
    }
    ze_kernel_export kernelExport = Win_BuildExport();
    g_window = {};

	g_window = linkPtr(kernelExport);
    if (g_window.Init == NULL)
    {
        Win_Warning("Unexpected result from module link", "Warning");
        return 1;
    }
    ErrorCode err = g_window.Init();
    if (err != ZE_ERROR_NONE)
    {
        Win_Warning("Error initialising window", "Warning");
        return 1;
    }

    // Window is okay. Begin App
    AppThread_Init();

    MainLoop();
    
    //Win32_Warning("window module says 666", "Okay");

    return 0;
}

#endif // WIN64_ZE_KERNEL_CPP
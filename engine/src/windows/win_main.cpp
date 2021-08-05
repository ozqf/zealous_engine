#include "ze_windows.h"

internal int g_bConsoleInit = FALSE;
internal HWND consoleHandle;

// performance counts per second
// Read Quad Part for a complete 64 bit integer
static LARGE_INTEGER g_timerFrequency;
// used to calculate time since application startup.
// NEVER change once set!
static i64 g_clockBase;

// read in pre-tokenised command line
extern int __argc;
extern char** __argv;

ze_external void *Platform_Alloc(size_t size)
{
	return malloc(size);
}

ze_external void Platform_Free(void *ptr)
{
	free(ptr);
}

////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////
ze_external void Platform_Fatal(const char *msg)
{
	printf("FATAL: %s\n", msg);
	MessageBoxA(0, (LPCSTR)msg, (LPCSTR) "Error", MB_OK | MB_ICONINFORMATION);
	DebugBreak();
}

static void Win_Warning(const char *msg)
{
	printf("WARNING: %s\n", msg);
	MessageBoxA(0, (LPCSTR)msg, (LPCSTR) "Warning", MB_OK | MB_ICONINFORMATION);
}

ze_external void Platform_DebugBreak()
{
	DebugBreak();
}

////////////////////////////////////////////////////////
// Timing
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

ze_external f64 Platform_QueryClock()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	i64 ticksSinceStart = counter.QuadPart - g_clockBase;
	f64 secondsSinceStart =
		(f64)((f64)ticksSinceStart / (f64)g_timerFrequency.QuadPart);
	//printf("%f seconds since startup\n", secondsSinceStart);
	return secondsSinceStart;
}

ze_external void Platform_Sleep(i32 milliSeconds)
{
	Sleep(milliSeconds);
}

////////////////////////////////////////////////////////
// I/O
////////////////////////////////////////////////////////
ze_external ZEBuffer Platform_StageFile(char* path)
{
	FILE* f;
	errno_t err = fopen_s(&f, path, "rb");
	if (err != 0)
	{
		printf("!Couldn't open file at %s\n", path);
		return {};
	}
	fseek(f, 0, SEEK_END);
	i32 size = ftell(f);
	fseek(f, 0, SEEK_SET);
	ZEBuffer buf = Buf_FromMalloc(Platform_Alloc, size);
	fread_s(&buf.start, size, 1, size, f);
	buf.cursor = buf.start + size;
	fclose(f);
	printf("Read %.3fKB from %s\n", (f32)size / 1024.f, path);
	return buf;
}

////////////////////////////////////////////////////////
// Console
////////////////////////////////////////////////////////
static void InitConsole()
{
	if (g_bConsoleInit)
	{
		return;
	}
	g_bConsoleInit = TRUE;
	// init live debug console
	FILE *stream;
	AllocConsole();
	freopen_s(&stream, "conin$", "r", stdin);
	freopen_s(&stream, "conout$", "w", stdout);
	freopen_s(&stream, "conout$", "w", stderr);
	consoleHandle = GetConsoleWindow();
	MoveWindow(consoleHandle, 1, 1, 680, 600, 1);
	printf("[%s] Console initialized.\n", __FILE__);
}

int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
	zErrorCode err;
	//////////////////////////////////////////
	// "pre-init" - setup config and read command line
	err = ZE_InitConfig(lpCmdLine, (const char **)__argv, __argc);
	ZE_ASSERT(err == ZE_ERROR_NONE, "Error initialising config")
	i32 tokenIndex = ZCFG_FindParamIndex("-c", "--console", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		InitConsole();
	}
	tokenIndex = ZCFG_FindParamIndex("-l", "--log", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		printf("Init log file...\n");
	}
	tokenIndex=  ZCFG_FindParamIndex("--pauseonstart", "--pauseonstart", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		Win_Warning("Pause on start!\n");
	}

	// init proper
	Win_InitTimer();
	ZE_Init();
	ZWindow_Init();
	ZE_StartLoop();
	printf("Done!\n");
	// Sleep(3000);
	// printf("Stopping!\n");
	// Sleep(200);
	return 0;
}

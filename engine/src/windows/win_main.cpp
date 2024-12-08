#include "ze_windows.h"

#include <userenv.h> // for SHGetFolderPath constants

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

internal int g_bConsoleInit = FALSE;
internal HWND consoleHandle;
internal HMODULE g_appDLL;
internal char g_exePath[MAX_PATH];

#define MAX_CUSTOM_MODULES 16
internal HMODULE g_customModules[MAX_CUSTOM_MODULES];
static i32 g_nextCustomerModule = 0;

// performance counts per second
// Read Quad Part for a complete 64 bit integer
static LARGE_INTEGER g_timerFrequency;
// used to calculate time since application startup.
// NEVER change once set!
static i64 g_clockBase;

#define MAX_CRASH_DUMP_FUNCTIONS 32
ze_internal ZE_CrashDumpFunction g_crashDumpFunctions[MAX_CRASH_DUMP_FUNCTIONS];
ze_internal i32 g_numCrashDumpFunctions = 0;

// read in pre-tokenised command line
extern int __argc;
extern char** __argv;

ze_external void Platform_Fatal(const char *msg);
static void Win_Warning(const char *msg);

ze_external void *Platform_Alloc(zeSize size)
{
	return malloc(size);
}

ze_external void *Platform_Realloc(void* ptr, zeSize size)
{
	return realloc(ptr, size);
}

ze_external void Platform_Free(void *ptr)
{
	free(ptr);
}

ze_internal void PrintExecutablePath()
{
	//WCHAR path[MAX_PATH];
	//GetModuleFileNameW(NULL, path, MAX_PATH);
	//printf("%ls\n", path);

	memset(g_exePath, 0, MAX_PATH);

	//char path[MAX_PATH];
	GetModuleFileNameA(NULL, g_exePath, MAX_PATH);
	// this gives us the exe name too which we don't care about
	// we just want a base path.
	zeSize len = ZStr_Len(g_exePath);
	for (zeSize i = len; i >= 0; --i)
	{
		char c = g_exePath[i];
		if (c != '\\')
		{
			g_exePath[i] = '\0';
		}
		else
		{
			break;
		}
	}
	
	printf("%s\n", g_exePath);
}

////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////

ze_internal void RegisterCrashDumpFunction(ZE_CrashDumpFunction fn)
{
	if (g_numCrashDumpFunctions >= MAX_CRASH_DUMP_FUNCTIONS)
	{
		// oops
		Win_Warning("No capacity for crash dump function");
		return;
	}
	g_crashDumpFunctions[g_numCrashDumpFunctions] = fn;
	g_numCrashDumpFunctions++;
}

ze_internal void CrashDump()
{
	printf("Begin crash dump\n");
	for (i32 i = 0; i < g_numCrashDumpFunctions; ++i)
	{
		g_crashDumpFunctions[i]();
	}
}

ze_external void Platform_Fatal(const char *msg)
{
	printf("FATAL: %s\n", msg);
	// so crash dump might err... crash, so pop up an error box first
	MessageBoxA(0, (LPCSTR)msg, (LPCSTR) "Error - pending crash dump", MB_OK | MB_ICONINFORMATION);
	CrashDump();
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
// Game DLL link
////////////////////////////////////////////////////////
internal ZGame_LinkupFunction* Win_LinkToGameDLL(char* customDir)
{
	return ZGame_Linkup;
	#if 0
	char targetPath[255];
	i32 strLen = sprintf_s(targetPath, 255, "%s\\%s", customDir, ZGAME_DLL_NAME);
	ZGame_LinkupFunction* linkUpPtr = NULL;
	// char* targetPath = "base\\game.dll";
	printf("Linking to game dll at \"%s\"\n", targetPath);
	g_appDLL = LoadLibraryA(targetPath);
	if (g_appDLL == NULL)
	{
		printf("\tApp DLL not found, loading fallback\n");
		return &ZGame_StubLinkup;
	}
	linkUpPtr = (ZGame_LinkupFunction*)GetProcAddress(g_appDLL, ZGAME_LINKUP_FUNCTION_NAME);
	if (linkUpPtr == NULL)
	{
		printf("\tFailed to find linking function in game dll, loading fallback\n");
		return &ZGame_StubLinkup;
	}
	return linkUpPtr;
	#endif
}

////////////////////////////////////////////////////////
// Game DLL link
////////////////////////////////////////////////////////

internal void* GetCustomModuleProcAddress(i32 moduleIndex, char* funcName)
{
	if (moduleIndex < 0 || moduleIndex >= g_nextCustomerModule)
	{
		printf("Module index %d is not out of bounds\n", moduleIndex);
		return NULL;
	}
	HMODULE dll = g_customModules[moduleIndex];
	void* ptr = GetProcAddress(dll, funcName);
	if (ptr == NULL)
	{
		printf("Failed to find function %s in module index %d\n", funcName, moduleIndex);
	}
	return ptr;
}

// returns index of hmodule or -1 if failed
internal i32 LinkToCustomModule(char* dllName)
{
	char targetPath[255];
	i32 strLen = sprintf_s(targetPath, 255, dllName);
	i32 i = g_nextCustomerModule;
	g_customModules[i] = LoadLibraryA(targetPath);
	if (g_customModules[i] == NULL)
	{
		printf("\tFailed to link to DLL %s\n", dllName);
		return -1;
	}
	g_nextCustomerModule += 1;
	return i;
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
ze_external ZEBuffer Platform_StageFile(const char* relativePath)
{
	// construct a full path relative to exe otherwise will use the launch path
	char path[MAX_PATH];
	char* end = path + MAX_PATH;
	memset(path, 0, MAX_PATH);
	char* cursor = path;
	cursor += ZE_COPY(g_exePath, cursor, ZStr_LenNoTerminator(g_exePath));
	cursor += ZE_COPY("\\", cursor, ZStr_LenNoTerminator("\\"));
	cursor += ZE_COPY(relativePath, cursor, ZStr_LenNoTerminator(relativePath));
	//cursor = strcpy_s(cursor, MAX_PATH,  g_exePath);
	//cursor = strcpy(cursor, "\\");
	//cursor = strcpy(cursor, relativePath);

	FILE* f;
	errno_t err = fopen_s(&f, path, "rb");
	if (err != 0)
	{
		printf("!Couldn't open file at %s\n", path);
		return {};
	}
	fseek(f, 0, SEEK_END);
	zeSize size = ftell(f);
	fseek(f, 0, SEEK_SET);
	ZEBuffer buf = Buf_FromMalloc(Platform_Alloc, size);
	//fread_s(&buf.start, size, size, 1, f);
	fread(buf.start, size, 1, f);
	buf.cursor = buf.start + size;
	fclose(f);
	printf("Read %.3fKB from %s\n", (f32)size / 1024.f, path);
	return buf;
}

ze_internal zErrorCode Platform_WriteFile(
	const char* path,
	void* data,
	zeSize numBytes)
{
	FILE* f;
	errno_t err = fopen_s(&f, path, "wb");
	if (err != 0)
	{
		printf("!Couldn't open file at %s\n", path);
		return ZE_ERROR_UNKNOWN;
	}
	fwrite(data, numBytes, 1, f);
	fclose(f);
	return ZE_ERROR_NONE;
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

extern "C" int ZE_GetVersion()
{
	return 2;
}

////////////////////////////////////////////////////////
// Windows Entry Point
////////////////////////////////////////////////////////

extern "C" int ZE_Startup(char* lpCmdLine)
{
	InitConsole();
	printf("ZE init\n");
	Platform_Sleep(3000);
	return 0;
}

#if 1
extern "C" int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
#else
extern "C" int ZE_Startup_Full(char* lpCmdLine)
#endif

{
	zErrorCode err;
	//////////////////////////////////////////
	// "pre-init" - setup config and read command line
	err = ZE_InitConfig(lpCmdLine, (const char **)__argv, __argc);
	ZE_ASSERT(err == ZE_ERROR_NONE, "Error initialising config")
	i32 bInitConsole = false;
	i32 bInitLogFile = false;
	i32 tokenIndex = ZCFG_FindParamIndex("-c", "--console", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		InitConsole();
	}
	tokenIndex = ZCFG_FindParamIndex("-l", "--log", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		// TODO - err, write logging code and err, init a log file...?
		// printf("Init log file...\n");
	}
	tokenIndex =  ZCFG_FindParamIndex("--pauseonstart", "--pauseonstart", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		Win_Warning("Pause on start!\n");
	}
	tokenIndex = ZCFG_FindParamIndex("-g", "--game", 1);
	char *gameDirectory;
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		gameDirectory = ZCFG_GetParamByIndex(tokenIndex + 1);
		if (gameDirectory == NULL || ZStr_Len(gameDirectory) <= 1)
		{
			printf("Custom game directory name is null or empty\n");
			gameDirectory = ZGAME_BASE_DIRECTORY;
		}
		printf("Game dir: %s\n", gameDirectory);
	}
	else
	{
		printf("No custom game directory specified\n");
		gameDirectory = ZGAME_BASE_DIRECTORY;
	}

	PrintExecutablePath();
	
	// find 'known' folder for local storage
	/*
	// need to acquire a token for this to work it seems
	CHAR path[MAX_PATH];
	DWORD size = MAX_PATH;
	GetUserProfileDirectoryA(token_here, path, &size);
	printf("User folder: %s\n", path);
	*/

	/////////////////////////////////
	// init proper
	Win_InitTimer();

	ZSystem sys = {};
	sys.Malloc = Platform_Alloc;
	sys.Realloc = Platform_Realloc;
	sys.Free = Platform_Free;
	sys.QueryClock = Platform_QueryClock;
	sys.GetScreenInfo = Window_GetInfo;
	sys.Fatal = Platform_Fatal;
	sys.RegisterCrashDumpFunction = RegisterCrashDumpFunction;

	ZFileIO io = {};
	io.WriteFile = Platform_WriteFile;
	io.StageFile = Platform_StageFile;

	i32 zpg = LinkToCustomModule("zpg.dll");
	if (zpg >= 0)
	{
		printf("Found zpg dll\n");
		void* zpgInitFn = GetCustomModuleProcAddress(zpg, "ZPG_Init");
		if (zpgInitFn != NULL)
		{
			printf("Found zpg init function\n");
		}
		else
		{
			printf("Failed to find ZPG init function\n");
		}
	}
	else
	{
		printf("Failed to find zpg dll\n");
	}
	
	
	err = ZEngine_Init(sys, io, Win_LinkToGameDLL(gameDirectory));
	if (err != ZE_ERROR_NONE)
	{
		printf("Error in ZEngine_Init - aborting\n");
		Platform_Fatal("Error in zengine init");
		return 1;
	}
	ZWindow_Init();
	ZE_StartLoop();
	printf("Exited loop\n");
	tokenIndex =  ZCFG_FindParamIndex("--pauseonstop", "--pauseonstop", 0);
	if (tokenIndex != ZE_ERROR_BAD_INDEX)
	{
		Win_Warning("Pause on stop!\n");
	}
	return 0;
}

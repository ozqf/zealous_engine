#include "ze_windows.h"

internal int g_bConsoleInit = FALSE;

internal HWND consoleHandle;

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
	ZEBuffer buf = Buf_FromMalloc(Platform_Alloc(size), size);
	fread_s(&buf.start, size, 1, size, f);
	buf.cursor = buf.start + size;
	fclose(f);
	printf("Read %.3fKB from %s\n", (f32)size / 1024.f, path);
	return buf;
}

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

	// init proper
	ZE_Init();
	ZWindow_Init();
	ZE_StartLoop();
	printf("Done!\n");
	// Sleep(3000);
	// printf("Stopping!\n");
	// Sleep(200);
	return 0;
}

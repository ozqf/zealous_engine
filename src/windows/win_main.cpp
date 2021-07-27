#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <shellapi.h> // for parsing command line tokens
#include <stdio.h>
#include <stdlib.h>

#include "../../headers/zengine.h"

internal int g_bConsoleInit = FALSE;

internal HWND consoleHandle;

extern int __argc;
extern char** __argv;

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
	ZE_InitConfig(lpCmdLine, (const char **)__argv, __argc);
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
	printf("Done!\n");
	Sleep(2000);
	return 0;
}

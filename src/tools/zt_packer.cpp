
#include "../ze_common/ze_common_full.h"
#include "windows.h"

#define ZPACK_MAX_PATH 260

static i32 g_stack = 0;

static void ZPack_FindAllFilesInDir(const char* searchPath);

static void ZPack_FindAllFilesInDir(const char* searchPath)
{
	ZE_BUILD_STRING(path, ZPACK_MAX_PATH, "%s/*", searchPath);
	printf("Start path: %s\n", path);
	g_stack++;
	if (g_stack > 10)
	{
		printf("ABORT Recursion overflow at %d\n", g_stack);
		return;
	}
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return;
	}
	do
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ZStr_Compare(FindFileData.cFileName, ".") == 0) { return; }
			if (ZStr_Compare(FindFileData.cFileName, "..") == 0) { return; }
			//printf("<DIR> %s\n", FindFileData.cFileName);
			ZE_BUILD_STRING(newPath, ZPACK_MAX_PATH, "%s/%s",
				searchPath, FindFileData.cFileName);
			ZPack_FindAllFilesInDir(newPath);
		}
		else
		{
			printf("  %s \n", FindFileData.cFileName);
		}
	} while (FindNextFileA(hFind, &FindFileData) != 0);
	FindClose(hFind);
	//g_stack--;
}

extern "C" void ZPack_Test()
{
	printf("=== TEST ZEALOUS PACKER ===\n");
	//
	const i32 capacity = MegaBytes(2);
	ZEBuffer buf = Buf_FromMalloc(malloc(capacity), capacity);
	const char* path = "base";
	ZPack_FindAllFilesInDir(path);
}

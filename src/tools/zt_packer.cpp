
#include "../ze_common/ze_common_full.h"
#include "windows.h"

#define ZPACK_MAX_PATH 260
/*
-- Header --
PACK
50 (offset to file list)
3 (num files)

-- Strings --

-- Files --

-- File List --

*/

struct DataFileHeader
{
	char magic[4];
	i32 stringsOffset;
	i32 itemsOffset;
	i32 numItems;
};

struct DataFileItem
{
	i32 pathHash;
	i32 pathOffset;
	i32 fileOffset;
	i32 fileSize;
};

static void ZPack_WriteDataFile(const char* fileName, ZEBuffer* paths, i32 numPaths)
{
	DataFileItem* items = (DataFileItem*)malloc(numPaths * sizeof(DataFileItem));
	FILE* f;
	errno_t err = fopen_s(&f, fileName, "wb");
	if (f == NULL)
	{
		printf("FAILED to open %s for writing with code %d\n", fileName, err);
		return;
	}
	printf("Writing %d files into %s\n", numPaths, fileName);
	DataFileHeader header = {};
	header.numItems = 0;
	header.stringsOffset = ftell(f);

	// step over space for header
	fseek(f, sizeof(DataFileHeader), SEEK_SET);
	fwrite(paths->start, paths->Written(), 1, f);

	i32 nextItem = 0;
	u8* read = paths->start;
	u8* end = paths->cursor;
	while (read < end)
	{
		ZEIntern* intern = (ZEIntern*)read;
		i32 internOffset = read - paths->start;
		char* path = (char*)paths->GetAtOffset(intern->charsOffset);
		printf("Adding %d: %s\n", intern->hash, path);
		read += sizeof(ZEIntern) + intern->len;
		// copy whole other file into this one.
		FILE* source;
		err = fopen_s(&source, path, "rb");
		if (err == 0) { continue; }
		
		// record info
		header.numItems += 1;
		i32 i = nextItem++;
		items[i].fileOffset = ftell(f);
		items[i].pathHash = intern->hash;
		items[i].pathOffset = header.stringsOffset + internOffset;

		// measure
		fseek(source, 0, SEEK_END);
		items[i].fileSize = ftell(source);
		fseek(source, 0, SEEK_SET);

		// copy
		int c;
		while ((c = fgetc(source)) != EOF)
		{
			fputc(c, f);
		}
	}
	// write file table
	header.itemsOffset = ftell(f);
	for (i32 i = 0; i < header.numItems; ++i)
	{
		fwrite((void*)&items[i], sizeof(DataFileItem), 1, f);
	}

	fclose(f);
}

static i32 ZPack_FindAllFilesInDir(const char* searchPath, ZEBuffer* strings);

static i32 ZPack_FindAllFilesInDir(const char* searchPath, ZEBuffer* strings)
{
	i32 count = 0;
	ZE_BUILD_STRING(path, ZPACK_MAX_PATH, "%s/*", searchPath);
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return count;
	}
	do
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ZStr_Compare(FindFileData.cFileName, ".") != 0
				&& ZStr_Compare(FindFileData.cFileName, "..") != 0)
			{
				ZE_BUILD_STRING(newPath, ZPACK_MAX_PATH, "%s/%s",
					searchPath, FindFileData.cFileName);
				count += ZPack_FindAllFilesInDir(newPath, strings);
			}
		}
		else
		{
			count++;
			// chop off the first directory in path, if there is one
			if (ZStr_CountSpecificChar(searchPath, '/') > 0)
			{
				ZE_BUILD_STRING(fullPath, ZPACK_MAX_PATH, "%s/%s",
					ZStr_ReadToChar((char*)searchPath, '/'), FindFileData.cFileName);
				ZEInternString(NULL, strings, fullPath);
			}
			else
			{
				ZEInternString(NULL, strings, FindFileData.cFileName);
			}
		}
	} while (FindNextFileA(hFind, &FindFileData) != 0);
	FindClose(hFind);
	return count;
}

extern "C" void ZPack_Test()
{
	printf("=== TEST ZEALOUS PACKER ===\n");
	const i32 capacity = MegaBytes(2);
	ZEBuffer buf = Buf_FromMalloc(malloc(capacity), capacity);
	const char* path = "base";
	i32 numFiles = ZPack_FindAllFilesInDir(path, &buf);
	printf("Found %d files in %s\n", numFiles, path);
	printf("%d path bytes interned\n", buf.Written());
	ZPack_WriteDataFile("base.dat", &buf, numFiles);
	// u8* read = buf.start;
	// u8* end = buf.cursor;
	// while (read < end)
	// {
	// 	ZEIntern* intern = (ZEIntern*)read;
	// 	printf("%d: %s\n", intern->hash, buf.GetAtOffset(intern->charsOffset));
	// 	read += sizeof(ZEIntern) + intern->len;
	// }
}

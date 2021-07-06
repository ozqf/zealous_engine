
#include "../../headers/common/ze_common_full.h"
#include "windows.h"

static void ZPack_PrintStrings(ZEBuffer* strings)
{
	u8* read = strings->start;
	u8* end = strings->cursor;
	while (read < end)
	{
		ZEIntern* intern = (ZEIntern*)read;
		printf("%d: %s\n", intern->hash, strings->GetAtOffset(intern->charsOffset));
		read += sizeof(ZEIntern) + intern->len;
	}
}

static void ZPack_WriteDataFile(
	const char* sourceDir,
	const char* outputName,
	ZEBuffer* paths,
	i32 numPaths)
{
	ZE_BUILD_STRING(fileName, Z_MAX_PATH, "%s.dat", outputName);
	
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
	header.magic[0] = 'P';
	header.magic[1] = 'A';
	header.magic[2] = 'C';
	header.magic[3] = 'K';
	header.version = 1;
	header.numItems = 0;

	// step over space for header
	fseek(f, sizeof(DataFileHeader), SEEK_SET);
	header.stringsOffset = ftell(f);
	header.stringBytes = paths->Written();
	
	// copy path strings
	fwrite(paths->start, paths->Written(), 1, f);

	i32 nextItem = 0;
	u8* read = paths->start;
	u8* end = paths->cursor;
	while (read < end)
	{
		ZEIntern* intern = (ZEIntern*)read;
		i32 internOffset = read - paths->start;
		char* path = (char*)paths->GetAtOffset(intern->charsOffset);
		
		read += sizeof(ZEIntern) + intern->len;
		
		// build full path
		ZE_BUILD_STRING(fullFilePath, Z_MAX_PATH, "%s/%s", sourceDir, path);
		// copy whole other file into this one.
		FILE* source;
		err = fopen_s(&source, fullFilePath, "rb");
		if (err != 0)
		{
			printf("Error %d opening file %s\n", err, fullFilePath);
			continue;
		}
		
		// record info
		header.numItems += 1;
		i32 i = nextItem++;
		items[i].fileOffset = ftell(f);
		items[i].pathHash = intern->hash;
		items[i].pathOffset = internOffset;
		
		// measure
		fseek(source, 0, SEEK_END);
		items[i].fileSize = ftell(source);
		fseek(source, 0, SEEK_SET);
		printf("Adding %d: %s (%dKB)\n",
			intern->hash, fullFilePath, items[i].fileSize / 1024);
		// copy
		#if 1 // why does this method work then?
		int c;
		while ((c = fgetc(source)) != EOF)
		{
			fputc(c, f);
		}
		#endif
		fclose(source);
	}
	// write file table
	header.itemsOffset = ftell(f);
	for (i32 i = 0; i < header.numItems; ++i)
	{
		fwrite((void*)&items[i], sizeof(DataFileItem), 1, f);
	}
	free(items);
	
	// write header
	fseek(f, 0, SEEK_SET);
	fwrite(&header, sizeof(DataFileHeader), 1, f);
	fseek(f, 0, SEEK_END);
	i32 totalBytes = ftell(f);
	printf("Finish: %d bytes, strings offset %d, items offset %d\n",
		totalBytes, header.stringsOffset, header.itemsOffset);
	fclose(f);
}

static i32 ZPack_FindAllFilesInDir(const char* searchPath, ZEBuffer* strings);

static i32 ZPack_FindAllFilesInDir(const char* searchPath, ZEBuffer* strings)
{
	i32 count = 0;
	ZE_BUILD_STRING(path, Z_MAX_PATH, "%s/*", searchPath);
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
				ZE_BUILD_STRING(newPath, Z_MAX_PATH, "%s/%s",
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
				ZE_BUILD_STRING(fullPath, Z_MAX_PATH, "%s/%s",
					ZStr_ReadToChar((char*)searchPath, '/'), FindFileData.cFileName);
				ZStr_Intern(NULL, strings, fullPath);
			}
			else
			{
				ZStr_Intern(NULL, strings, FindFileData.cFileName);
			}
		}
	} while (FindNextFileA(hFind, &FindFileData) != 0);
	FindClose(hFind);
	return count;
}

static void ZPack_ScanDataFile(const char* path)
{
	printf("Scanning data file %s\n", path);
	FILE* f;
	errno_t err = fopen_s(&f, path, "rb");
	if (err != 0)
	{
		printf("ERROR %d opening %s for scan\n", err, path);
		return;
	}
	fseek(f, 0, SEEK_END);
	i32 numBytes = ftell(f);
	fseek(f, 0, SEEK_SET);
	DataFileHeader header;
	fread_s(&header, sizeof(DataFileHeader), sizeof(DataFileHeader), 1, f);
	printf("%c%c%c%c ver %d, %d items, %d string bytes\n",
		header.magic[0],
		header.magic[1],
		header.magic[2],
		header.magic[3],
		header.version,
		header.numItems,
		header.stringBytes
	);
	i32 stringCursor = header.stringsOffset;
	i32 itemsCursor = header.itemsOffset;
	fseek(f, header.stringsOffset, SEEK_SET);
	ZEBuffer buf = Buf_FromMalloc(malloc(header.stringBytes), header.stringBytes);
	fread_s(buf.cursor, buf.capacity, buf.capacity, 1, f);
	buf.cursor += header.stringBytes;
	//ZPack_PrintStrings(&buf);
	fseek(f, header.itemsOffset, SEEK_SET);
	for (i32 i = 0; i < header.numItems; ++i)
	{
		DataFileItem item;
		fread_s(&item, sizeof(DataFileItem), sizeof(DataFileItem), 1, f);
		ZEIntern* intern = (ZEIntern*)buf.GetAtOffset(item.pathOffset);
		char* filePath = (char*)buf.GetAtOffset(intern->charsOffset);
		printf("%s. %d bytes at %d\n", filePath, item.fileSize, item.fileOffset);
	}
	free(buf.start);
	fclose(f);
}

internal void ZPack_Pack(const char* dirName, const char* outputName)
{
	printf("Packing %s into %s.dat\n", dirName, outputName);
	const i32 capacity = MegaBytes(2);
	ZEBuffer buf = Buf_FromMalloc(malloc(capacity), capacity);
	i32 numFiles = ZPack_FindAllFilesInDir(dirName, &buf);
	printf("Found %d files in %s\n", numFiles, dirName);
	printf("%d path bytes interned\n", buf.Written());
	ZPack_WriteDataFile(dirName, outputName, &buf, numFiles);
}

extern "C" void ZPack_Run(i32 argc, char** argv)
{
	printf("=== ZEALOUS PACKER ===\n");
	i32 tokenIndex;
	tokenIndex = ZE_FindParamIndex((char**)argv, argc, "-scan", 1);
	if (tokenIndex >= 0)
	{
		printf("Scan token at %d\n", tokenIndex);
		ZPack_ScanDataFile(argv[tokenIndex + 1]);
		return;
	}
	tokenIndex = ZE_FindParamIndex((char**)argv, argc, "-pack", 2);
	if (tokenIndex >= 0)
	{
		ZPack_Pack(argv[tokenIndex + 1], argv[tokenIndex + 2]);
		return;
	}
	printf("Packer found no action to perform...\n");
	printf("Examples: Pack contents of /base/ into file pak01.dat\n");
	printf("\tzetools packer -pack base pak01\n");
	printf("Examples: Scan contents of pak01.dat\n");
	printf("\tzetools packer -scan pak01.dat\n");
}

extern "C" void ZPack_Test()
{
	printf("=== TEST ZEALOUS PACKER ===\n");
	const i32 capacity = MegaBytes(2);
	ZEBuffer buf = Buf_FromMalloc(malloc(capacity), capacity);
	const char* dirName = "base";
	i32 numFiles = ZPack_FindAllFilesInDir(dirName, &buf);
	printf("Found %d files in %s\n", numFiles, dirName);
	printf("%d path bytes interned\n", buf.Written());
	ZPack_WriteDataFile(dirName, dirName, &buf, numFiles);
	ZPack_ScanDataFile("base.dat");
}

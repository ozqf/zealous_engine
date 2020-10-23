
#include "../ze_common/ze_common.h"

#include <windows.h>
#include <stdio.h>

// file handle saves record of an
// open FILE pointer or a heap allocated
// file.
struct ZEFileIOHandle
{
    i32 id; // only > 0 is valid
    i32 bReadMode;
    u8 *readAlloc;
    i32 numBytes;
    FILE *f;
};

// Max handles open for reading or writing at once.
#define WIN_MAX_FILE_IO_HANDLES 32
#define WIN_NULL_FILE_IO_HANDLE 0

#define WIN_MAX_DATA_FILES 128

struct DataFileHandle
{
    FILE *f;
    ZEBuffer strings;
    DataFileHeader header;
};

struct DataFileItemLocation
{
    i32 fileIndex;
    DataFileItem item;
};

internal ZEFileIOHandle g_fileHandles[WIN_MAX_FILE_IO_HANDLES];
internal i32 g_nextFileIOHandle = 1;
internal DataFileHandle g_dataFiles[WIN_MAX_DATA_FILES];
internal i32 g_nextDataFile = 0;

internal i32 WinIO_FindInDataFiles(const char* path, DataFileItemLocation* result)
{
    i32 hash = ZE_Hash_djb2((u8*)path);
    // data files read in alphabetically, so read order backwards
    for (i32 i = g_nextDataFile - 1; i >= 0; --i)
    {
        DataFileHandle* h = &g_dataFiles[i];
        // ZEIntern* intern = ZStr_FindInternLinear(&h->strings, path);
        fseek(h->f, h->header.itemsOffset, SEEK_SET);
        for (i32 j = 0; j < h->header.numItems; ++j)
        {
            DataFileItem item;
            fread_s(&item, sizeof(DataFileItem), sizeof(DataFileItem), 1, h->f);
            if (item.pathHash == hash)
            {
                result->fileIndex = i;
                result->item = item;
                return YES;
            }
        }
    }
    return NO;
}

internal i32 WinIO_OpenDataFile(const char *path, DataFileHandle *result)
{
    errno_t err = fopen_s(&result->f, path, "rb");
    if (err != 0)
    {
        printf("Failed to open data file %s\n", path);
        return NO;
    }
    fread_s(&result->header, sizeof(DataFileHeader), sizeof(DataFileHeader), 1, result->f);
    if (
        result->header.magic[0] != 'P'
        || result->header.magic[1] != 'A'
        || result->header.magic[2] != 'C'
        || result->header.magic[3] != 'K')
    {
        printf("ERROR Bad magic number in %s\n", path);
        fclose(result->f);
        return NO;
    }
    printf("Opened data file %s (%d items)\n", path, result->header.numItems);
    return YES;
}

internal void WinIO_GatherDataFiles(const char *dir)
{
    ZE_BUILD_STRING(path, 260, "%s/*", dir)
    // Scan
    WIN32_FIND_DATAA FindFileData;
    HANDLE hFind = FindFirstFileA(path, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            i32 len = ZStr_Len(FindFileData.cFileName);
            if (ZStr_CheckExtension(FindFileData.cFileName, len, ".dat"))
            {
                ZE_BUILD_STRING(dataFilePath, 260, "%s/%s", dir, FindFileData.cFileName);
                DataFileHandle *handle = &g_dataFiles[g_nextDataFile];
                if (WinIO_OpenDataFile(dataFilePath, handle))
                {
                    g_nextDataFile++;
                    if (g_nextDataFile >= WIN_MAX_FILE_IO_HANDLES)
                    {
                        break;
                    }
                }
            }
        }
    } while (FindNextFileA(hFind, &FindFileData) != 0);
    FindClose(hFind);
}

/**
 * Returns IO handle id
 */
internal i32 WinIO_OpenFile(const char *path, i32 bRead)
{
    i32 id = WIN_NULL_FILE_IO_HANDLE;
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        if (g_fileHandles[i].id != WIN_NULL_FILE_IO_HANDLE)
        {
            continue;
        }

        FILE *f = NULL;
        errno_t err;
        if (bRead == YES)
        {
            err = fopen_s(&f, path, "rb");
        }
        else
        {
            err = fopen_s(&f, path, "wb");
        }

        if (f == NULL)
        {
            printf("WIN Failed with code %d opening %s for writing\n", err, path);
            return WIN_NULL_FILE_IO_HANDLE;
        }
        id = g_nextFileIOHandle++;
        g_fileHandles[i].id = id;
        g_fileHandles[i].bReadMode = bRead;
        g_fileHandles[i].f = f;
        return id;
    }
    printf("WIN No free handles to open %s for writing\n", path);
    return WIN_NULL_FILE_IO_HANDLE;
}

internal void WinIO_CloseFileHandle(i32 handle)
{
    if (handle <= WIN_NULL_FILE_IO_HANDLE)
    {
        return;
    }
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        ZEFileIOHandle *h = &g_fileHandles[i];
        if (h->id == handle)
        {
            // release assigned resources
            if (h->f != NULL)
            {
                i32 numBytes = ftell(h->f);
                printf("WIN Freeing file on handle %d (%d bytes)\n",
                       handle, numBytes);
                errno_t err = fclose(g_fileHandles[i].f);
                ZE_ASSERT(err == 0, "Error closing file handle");
            }
            if (h->readAlloc != NULL)
            {
                printf("WIN Freeing %d bytes on handle %d\n", h->numBytes, handle);
                free(h->readAlloc);
            }
            // recycle
            g_fileHandles[i] = {};
            return;
        }
    }
    printf("WIN no file handle %d to close\n", handle);
}

internal i32 WinIO_FilePosition(i32 handle)
{
    if (handle <= WIN_NULL_FILE_IO_HANDLE)
    {
        -1;
    }
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        if (g_fileHandles[i].id == handle)
        {
            ZE_ASSERT(g_fileHandles[i].f != NULL, "FilePosition - null file ptr");
            return ftell(g_fileHandles[i].f);
        }
    }
    return -1;
}

internal void WinIO_WriteToFile(i32 handle, u8 *bytes, i32 numBytes)
{
    if (handle <= WIN_NULL_FILE_IO_HANDLE)
    {
        return;
    }
    if (numBytes <= 0)
    {
        return;
    }
    ZE_ASSERT(bytes != NULL, "Win null bytes in WriteToFile")
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        if (g_fileHandles[i].id == handle)
        {
            ZE_ASSERT(g_fileHandles[i].bReadMode == NO,
                      "WriteToFile - Cannot write to file in read mode")
            ZE_ASSERT(g_fileHandles[i].f != NULL, "WriteToFile - null file ptr");
            fwrite(bytes, numBytes, 1, g_fileHandles[i].f);
            g_fileHandles[i].numBytes += numBytes;
            return;
        }
    }
    printf("WIN no file handle %d to write to\n", handle);
}

internal void WinIO_WritePadding(i32 handle, i32 numBytes, u8 value)
{
    if (handle <= WIN_NULL_FILE_IO_HANDLE)
    {
        return;
    }
    if (numBytes <= 0)
    {
        return;
    }
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        ZEFileIOHandle *h = &g_fileHandles[i];
        if (h->id == handle)
        {
            ZE_ASSERT(h->f != NULL, "WritePadding file ptr is null")
            ZE_ASSERT(h->bReadMode == NO, "WritePadding file is not in write mode")
            fwrite(&value, 1, numBytes, h->f);
            h->numBytes += numBytes;
        }
    }
}

internal void WinIO_WriteToFileAtOffset(i32 handle, u8 *bytes, i32 numBytes, i32 offset)
{
    if (handle <= WIN_NULL_FILE_IO_HANDLE)
    {
        return;
    }
    if (numBytes <= 0)
    {
        return;
    }
    if (offset < 0)
    {
        return;
    }
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        ZEFileIOHandle *h = &g_fileHandles[i];
        if (h->id == handle)
        {
            ZE_ASSERT(offset < h->numBytes, "WriteToFileAtOffset out of bounds")
            ZE_ASSERT(h->f != NULL, "WriteToFileAtOffset file ptr is null")
            ZE_ASSERT(h->bReadMode == NO, "WriteToFileAtOffset file is not in write mode")
            i32 current = ftell(h->f);
            fseek(h->f, offset, SEEK_SET);
            printf("WIN writing %d bytes at pos %d into handle %d\n",
                   numBytes, offset, h->id);
            fwrite(bytes, numBytes, 1, h->f);
            // update size, incase the wrote pass the previous end of file
            fseek(h->f, 0, SEEK_END);
            h->numBytes = ftell(h->f);
            // reset write position
            fseek(h->f, current, SEEK_SET);
        }
    }
}

internal void WinIO_FreeStagedFile(void* ptr)
{
    // TODO record staged files and check release here
    free(ptr);
}

internal ErrorCode WinIO_StageFile(const char *path, i32 bOnlyPacks, ZEBuffer *result)
{
    if (path == NULL)
    {
        return WIN_NULL_FILE_IO_HANDLE;
    }
    if (result == NULL)
    {
        return WIN_NULL_FILE_IO_HANDLE;
    }
    DataFileItemLocation itemLoc;
    if (WinIO_FindInDataFiles(path, &itemLoc))
    {
        printf("Found %s at %d in data file %d\n",
            path, itemLoc.item.fileOffset, itemLoc.fileIndex);
        FILE* f = g_dataFiles->f;
        fseek(f, itemLoc.item.fileOffset, SEEK_SET);
        i32 size = itemLoc.item.fileSize;
        *result = Buf_FromMalloc(malloc(size), size);
        fread_s(result->start, size, size, 1, f);
        result->cursor = result->start + size;
        return ZE_ERROR_NONE;
    }
    else
    {
        printf("File not found in data files, looking at disk\n");
        FILE* f;
        errno_t err;
        err = fopen_s(&f, path, "rb");
        if (f == NULL)
        {
            printf("WIN err %d staging file %s from disk\n", err, path);
            return ZE_ERROR_NOT_FOUND;
        }
        fseek(f, 0, SEEK_END);
        size_t numBytes = ftell(f);
        fseek(f, 0, SEEK_SET);
        *result = Buf_FromMalloc(malloc(numBytes), numBytes);
        fread_s(result->start, numBytes, numBytes, 1, f);
        result->cursor = result->start + numBytes;
        fclose(f);
        return ZE_ERROR_NONE;
    }
#if 0
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        if (g_fileHandles[i].id != WIN_NULL_FILE_IO_HANDLE)
        {
            continue;
        }
        ZEFileIOHandle *h = &g_fileHandles[i];
        FILE *f = NULL;
        errno_t err;
        err = fopen_s(&f, path, "rb");
        if (f == NULL)
        {
            printf("WIN err %d staging file %s\n", err, path);
            return WIN_NULL_FILE_IO_HANDLE;
        }
        h->id = g_nextFileIOHandle++;
        h->bReadMode = YES;
        fseek(f, 0, SEEK_END);
        h->numBytes = ftell(f);
        if (h->numBytes == 0)
        {
            printf("WIN no bytes in file %s\n", path);
            return ZE_ERROR_BAD_SIZE;
        }
        fseek(f, 0, SEEK_SET);
        h->readAlloc = (u8 *)malloc(h->numBytes);
        *result = Buf_FromMalloc(h->readAlloc, h->numBytes);
        fread_s(result->start, h->numBytes, h->numBytes, 1, f);
        result->cursor = result->start + h->numBytes;
        fclose(f);
        h->f = NULL;
        printf("WIN - staged file %s (%d bytes)\n", path, result->Written());
        return h->id;
    }
    printf("WIN no file handle to stage %s\n", path);
    return WIN_NULL_FILE_IO_HANDLE;
#endif
}

internal void WinIO_DumpHandles()
{
    printf("=== File handles ===\n");
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        ZEFileIOHandle *h = &g_fileHandles[i];
        if (h->id == WIN_NULL_FILE_IO_HANDLE)
        {
            continue;
        }
        printf("%d: id %d rmode %d, fileptr %d, mem %d bytes at %d\n",
               i, h->id, h->bReadMode, (i32)h->f, h->numBytes, (i32)h->readAlloc);
    }
}

internal ZEFileIO WinIO_Init(const char *baseDir, const char *appDir)
{
    ZE_SET_ZERO(g_fileHandles, sizeof(ZEFileIOHandle) * WIN_MAX_FILE_IO_HANDLES);

    if (ZStr_Compare(baseDir, appDir) == 0)
    {
        appDir = NULL;
    }

    WinIO_GatherDataFiles(baseDir);
    printf("Opened %d data files\n", g_nextDataFile);

    ZEFileIO files = {};
    files.OpenFile = WinIO_OpenFile;
    files.CloseFile = WinIO_CloseFileHandle;
    files.FilePosition = WinIO_FilePosition;
    files.WriteToFile = WinIO_WriteToFile;
    files.StageFile = WinIO_StageFile;
    files.FreeStagedFile = WinIO_FreeStagedFile;
    files.WritePadding = WinIO_WritePadding;
    files.WriteToFileAtOffset = WinIO_WriteToFileAtOffset;
    return files;
}

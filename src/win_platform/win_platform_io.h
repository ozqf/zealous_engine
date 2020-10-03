
#include "../ze_common/ze_common.h"

#include <windows.h>
#include <stdio.h>


struct ZEFileIOHandle
{
    i32 id; // only > 0 is valid
    i32 bReadMode;
    u8* readAlloc;
    i32 numBytes;
    FILE* f;
};

// Max handles open for reading or writing at once.
#define WIN_MAX_FILE_IO_HANDLES 32
#define WIN_NULL_FILE_IO_HANDLE 0

internal i32 g_nextFileIOHandle = 1;

internal ZEFileIOHandle g_fileHandles[WIN_MAX_FILE_IO_HANDLES];

internal void WinIO_Init()
{
    ZE_SET_ZERO(g_fileHandles, sizeof(ZEFileIOHandle) * WIN_MAX_FILE_IO_HANDLES);
}

/**
 * Returns IO handle id
 */
internal i32 WinIO_OpenFile(const char* path, i32 bRead)
{
    i32 id = WIN_NULL_FILE_IO_HANDLE;
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        if (g_fileHandles[i].id != WIN_NULL_FILE_IO_HANDLE)
        { continue; }

        FILE* f = NULL;
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
    if (handle <= WIN_NULL_FILE_IO_HANDLE) { return; }
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        if (g_fileHandles[i].id == handle)
        {
            errno_t err = fclose(g_fileHandles[i].f);
            ZE_ASSERT(err == 0, "Error closing file handle");
            g_fileHandles[i] = {};
            return;
        }
    }
    printf("WIN no file handle %d to close\n", handle);
}

internal void WinIO_WriteToFile(i32 handle, u8* bytes, i32 numBytes)
{
    if (handle <= WIN_NULL_FILE_IO_HANDLE) { return; }
    for (i32 i = 0; i < WIN_MAX_FILE_IO_HANDLES; ++i)
    {
        if (g_fileHandles[i].id == handle)
        {
            ZE_ASSERT(g_fileHandles[i].bReadMode == NO,
                "Cannot write to file in read mode")
            fwrite(bytes, numBytes, 1, g_fileHandles[i].f);
            g_fileHandles[i].numBytes += numBytes;
            return;
        }
    }
    printf("WIN no file handle %d to write to\n", handle);
}

internal void WinIO_StageFile(char* path)
{

}

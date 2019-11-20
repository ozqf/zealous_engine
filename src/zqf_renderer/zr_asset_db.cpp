#ifndef ZR_ASSET_DB_CPP
#define ZR_ASSET_DB_CPP

#include "zr_asset_db.h"

struct ZRTextureHandle
{
    char* fileName;
    void* data;
    i32 dataSize;
    i32 apiHandle;
};

#define ZR_ASSET_DB_MAX_HANDLES 256

internal ZRTextureHandle g_textures[ZR_ASSET_DB_MAX_HANDLES];
internal i32 g_nextTexture = 0;

extern "C" i32 ZRDB_RegisterTexture(char* fileName, void* data, i32 dataSize, i32 apiHandle)
{
    i32 index = g_nextTexture++;
    ZRTextureHandle* handle = &g_textures[index];
    handle->fileName = fileName;
    handle->data = data;
    handle->dataSize = dataSize;
    handle->apiHandle = apiHandle;
    return index;
}

extern "C" i32 ZRDB_GetTexIndexByName(char* name)
{
    for (i32 i = 0; i < g_nextTexture; ++i)
    {
        if (ZE_CompareStrings(name, g_textures[i].fileName) == 0)
        {
            return i;
        }
    }
    return 0;
}

extern "C" i32 ZRDB_GetTexHandleByIndex(i32 index)
{
    if (index < 0 || index >= g_nextTexture) { return 0; }
    return g_textures[index].apiHandle;
}

#endif // ZR_ASSET_DB_CPP
#ifndef ZR_DB_TEXTURES_H
#define ZR_DB_TEXTURES_H

// Asset libraries:
#define STB_IMAGE_IMPLEMENTATION

#define STBI_MALLOC(sz) g_alloc.Allocate(sz)
#define STBI_REALLOC(p, sz) g_alloc.Realloc(p, sz)
#define STBI_FREE(p) g_alloc.Free(p)

#include "../../lib/stb_image.h"
#include "../../lib/openfbx/ofbx.h"

#include "zr_asset_db.h"

static i32 ZRDB_GetNumTextures(ZRAssetDB* assetDB)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    return db->numTextures;
}

static i32 ZRDB_RegisterTexture(
    ZRAssetDB* assetDB,
    char* fileName,
    ColourU32* data,
    i32 dataSize,
    i32 width,
    i32 height,
    i32 apiHandle)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    
    i32 index = db->numTextures++;
    printf("ZRDB - registered texture %d: %s, %d/%d, handle %d\n",
        index, fileName, width, height, apiHandle);
    ZRDBTexture* handle = &db->textures[index];
    *handle = {};
    handle->header.id = db->nextId;
    db->nextId++;
    handle->header.index = index;
    handle->header.fileName = fileName;
    handle->data = data;
    handle->dataSize = dataSize;
    handle->width = width;
    handle->height = height;
    handle->apiHandle = apiHandle;
    return index;
}

static i32 ZRDB_GetTextureIndexByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    for (i32 i = 0; i < db->numTextures; ++i)
    {
        if (ZStr_Compare(name, db->textures[i].header.fileName) == 0)
        {
            return i;
        }
    }
    printf("ZRDB FAIL - No tex %s\n", name);
    return 0;
}

static ZRDBTexture* ZRDB_GetTextureByIndex(ZRAssetDB* assetDB, i32 index)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    if (index < 0 || index >= db->numTextures) { index = 0; }
    return &db->textures[index];
}

static ZRDBTexture* ZRDB_GetTextureByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = ZRDB_GetTextureIndexByName(assetDB, name);
    return &db->textures[index];
}

static i32 ZRDB_GetTextureHandleByIndex(ZRAssetDB* assetDB, i32 index)
{
    ZRDBTexture* tex = ZRDB_GetTextureByIndex(assetDB, index);
    return tex->apiHandle;
}

static ColourU32* ZRDB_LoadTextureToHeap(
    ZEFileIO* files,
    char* path,
    i32 bVerbose,
    int* x,
    int* y,
    i32 bFlipY)
{
    ZEBuffer b;
    if (files->StageFile(path, NO, &b) != ZE_ERROR_NONE)
    {
        return NULL;
    }
    // Load to heap:
    i32 comp;
    stbi_set_flip_vertically_on_load(bFlipY);
    u8* tex = stbi_load_from_memory(
        b.start, b.capacity, x, y, &comp, STBI_rgb_alpha);
    files->FreeStagedFile(b.start);
    return (ColourU32*)tex;
}

static i32 ZRDB_UploadTexture(ZRDBTexture* tex)
{
    return ZE_ERROR_NOT_IMPLEMENTED;
}

static i32 ZRDB_LoadTexture(ZRAssetDB* assetDB, char* path, i32 bVerbose)
{
    printf("ZRDB load texture %s verbose: %d\n", path, bVerbose);
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
	ColourU32* pixels;
	i32 x, y;
	u32 handle = 0;
    pixels = ZRDB_LoadTextureToHeap(&db->files, path, bVerbose, &x, &y, YES);
    if (pixels == NULL)
    {
        printf("\tTex load failed!\n");
        return 0;
    }
	//db->uploader.UploadTexture(pixels, x, y, &handle);
    i32 dataSize = sizeof(ColourU32) * (x * y);
	ZRDB_RegisterTexture(assetDB, path, pixels, dataSize, x, y, handle);
    return 0;
}

#endif // ZR_DB_TEXTURES_H
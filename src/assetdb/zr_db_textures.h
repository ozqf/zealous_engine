#ifndef ZR_DB_TEXTURES_H
#define ZR_DB_TEXTURES_H

// Asset libraries:
#define STB_IMAGE_IMPLEMENTATION
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
    printf("ZRDB - registered texture %d: %s, handle %d\n",
        index, fileName, apiHandle);
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
        if (ZE_CompareStrings(name, db->textures[i].header.fileName) == 0)
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
    char* path, i32 bVerbose, int* x, int* y, i32 bFlipY)
{
    ZEByteBuffer b;
    ErrorCode err = ZRDB_StageRawFile(path, &b);
    if (err != ZE_ERROR_NONE)
    {
        return NULL;
    }
    
    // Load to heap:
    i32 comp;
    stbi_set_flip_vertically_on_load(bFlipY);
    u8* tex = stbi_load_from_memory(
        b.start, b.capacity, x, y, &comp, STBI_rgb_alpha);
    if (bVerbose == YES)
    { printf("Loaded img res %d, %d - comp %d\n", *x, *y, comp); }
    free(b.start);
    return (ColourU32*)tex;
}

static i32 ZRDB_UploadTexture(ZRDBTexture* tex)
{
    return ZE_ERROR_NOT_IMPLEMENTED;
}

static i32 ZRDB_LoadTexture(ZRAssetDB* assetDB, char* path, i32 bVerbose)
{
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
	ZEByteBuffer buf;
	if (ZRDB_StageRawFile(path, &buf) != ZE_ERROR_NONE)
	{
		return 0;
	}
	ColourU32* pixels;
	i32 x, y;
	u32 handle = 0;
    pixels = ZRDB_LoadTextureToHeap(path, bVerbose, &x, &y, YES);
	//db->uploader.UploadTexture(pixels, x, y, &handle);
	ZRDB_RegisterTexture(assetDB, path, pixels, buf.capacity, x, y, handle);
    return 0;
}

#endif // ZR_DB_TEXTURES_H
#ifndef ZR_DB_INTERNAL_H
#define ZR_DB_INTERNAL_H

#include "zr_asset_db.h"
#include "../ze_common/ze_common_full.h"

#define ZR_ASSET_DB_MAX_HANDLES 512

#define ZR_ASSET_DB_FIRST_ID 1

#define ZRDB_DEFAULT_MAT_INDEX 0

///////////////////////////////////////////////////////////////////////////
// Internal data structures
///////////////////////////////////////////////////////////////////////////

struct ZRAssetDBData
{
    // public head contains function pointers
    ZRAssetDB header;
    //ZRAssetUploader uploader;
    // external links
    ZEFileIO files;
    
    ZEBuffer strings;
	MallocList allocs;

    // unique serial number shared across all assets.
    i32 nextId;
    
    ZRDBBlob* blobs;
    i32 numBlobs;
    i32 maxBlobs;

    ZRDBTexture* textures;
    i32 numTextures;
    i32 maxTextures;

    ZRDBMesh* meshes;
    i32 numMeshes;
    i32 maxMeshes;

    ZRDBMaterial* materials;
    i32 numMaterials;
    i32 maxMaterials;
};

static i32 ZRDB_RegisterTexture(
    ZRAssetDB* assetDB, char* fileName, void* data, i32 dataSize, i32 width, i32 height, i32 apiHandle);
static ZRDBTexture* ZRDB_CreateBlankTexture(ZRAssetDB* handle, char* name, i32 w, i32 h, ColourU32 fill);

// Global for using it in stbi malloc/free etc
static ZEFileIO g_files;
static ZEAllocator g_alloc;

///////////////////////////////////////////////////////////////////////////
// Shared utility functions
///////////////////////////////////////////////////////////////////////////

static u32 ZRDB_MeasureFile(char* path)
{
    FILE* f;
    i32 err = fopen_s(&f, path, "rb");
    if (err != 0)
    {
        printf("FAILED: Could not open file \"%s\" for reading\n", path);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    fclose(f);
    return size;
}

#define ZRDB_CAST_TO_INTERNAL(assetDBHeader, newVarName) \
ZRAssetDBData*##newVarName##=##(ZRAssetDBData*)##assetDBHeader##;

#endif // ZR_DB_INTERNAL_H
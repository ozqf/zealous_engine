#ifndef ZR_DB_INTERNAL_H
#define ZR_DB_INTERNAL_H


#include "zr_asset_db.h"

#define ZR_ASSET_DB_MAX_HANDLES 512

#define ZR_ASSET_DB_FIRST_ID 1

///////////////////////////////////////////////////////////////////////////
// Internal data structures
///////////////////////////////////////////////////////////////////////////

struct ZRAssetDBData
{
    ZRAssetDB header;
    ZRAssetUploader uploader;

    i32 nextId;
    ZRDBTexture* textures;
    i32 numTextures;
    i32 maxTextures;
    ZRDBMesh* meshes;
    i32 numMeshes;
    i32 maxMeshes;
    ZRMaterial* materials;
    i32 numMaterials;
    i32 maxMaterials;
};


///////////////////////////////////////////////////////////////////////////
// Shared utility functions
///////////////////////////////////////////////////////////////////////////

/**
 * Load an entire file, unaltered, into memory.
 * Must be freed by the caller after use
 */
static i32 ZRDB_StageRawFile(char* path, ZEByteBuffer* dest)
{
    FILE* f;
    i32 err = fopen_s(&f, path, "rb");
    if (err != 0)
    {
        printf("FAILED: Could not open file \"%s\" for reading\n", path);
        return ZE_ERROR_NOT_FOUND;
    }
    fseek(f, 0, SEEK_END);
    i32 size = ftell(f);
    fseek(f, 0, SEEK_SET);
    void* mem = malloc(size);
    if (mem == NULL)
    {
        return ZE_ERROR_ALLOCATION_FAILED;
    }
    *dest = Buf_FromMalloc(mem, size);
    fread((void*)dest->start, dest->capacity, 1, f);
    fclose(f);
    //printf("  staged \"%s\" (%d KB)\n", size / 1024);
    return ZE_ERROR_NONE;
}

#define ZRDB_CAST_TO_INTERNAL(assetDBHeader, newVarName) \
ZRAssetDBData*##newVarName##=##(ZRAssetDBData*)##assetDBHeader##;


#endif // ZR_DB_INTERNAL_H
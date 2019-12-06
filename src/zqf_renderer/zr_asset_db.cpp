#ifndef ZR_ASSET_DB_CPP
#define ZR_ASSET_DB_CPP

#include "zr_asset_db.h"

#define ZR_ASSET_DB_MAX_HANDLES 512

///////////////////////////////////////////////////////////////////////////
// Internal data structures
///////////////////////////////////////////////////////////////////////////

struct ZRAssetDBData
{
    ZRAssetDB header;
    ZRAssetUploader uploader;

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

///////////////////////////////////////////////////////////////////////////
// Implementations
///////////////////////////////////////////////////////////////////////////

#include "zr_db_textures.h"
#include "zr_db_meshes.h"
#include "zr_db_materials.h"

///////////////////////////////////////////////////////////////////////////
// Create
///////////////////////////////////////////////////////////////////////////

extern "C" ZRAssetDB* ZRDB_Create(ZRAssetUploader uploader)
{
    ZRAssetDBData* db = (ZRAssetDBData*)malloc(sizeof(ZRAssetDBData));
    *db = {};
    // callbacks
    db->uploader = uploader;
    /////////////////////////////////////////////////////
    // functions

    // Get asset
    db->header.GetMeshByName = ZRDB_GetMeshByName;
    db->header.GetMeshHandleByName = ZRDB_GetMeshHandleByName;

    db->header.GetTextureByName = ZRDB_GetTextureByName;
    db->header.GetTextureHandleByIndex = ZRDB_GetTextureHandleByIndex;

    db->header.GetMaterialByName = ZRDB_GetMaterialByName;
    db->header.GetMaterialByIndex = ZRDB_GetMaterialByIndex;
    // Create
    db->header.CreateMaterial = ZRDB_CreateMaterial;
    // Load
    db->header.LoadMesh = ZRDB_LoadMesh;
    db->header.LoadMeshFromFBX = ZRDB_LoadMeshFromFBX;
    db->header.LoadTexture = ZRDB_LoadTexture;

    // store
    db->textures = (ZRDBTexture*)malloc(sizeof(ZRDBTexture) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxTextures = ZR_ASSET_DB_MAX_HANDLES;
    db->meshes = (ZRDBMesh*)malloc(sizeof(ZRDBMesh) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMeshes = ZR_ASSET_DB_MAX_HANDLES;
    db->materials = (ZRMaterial*)malloc(sizeof(ZRMaterial) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMaterials = ZR_ASSET_DB_MAX_HANDLES;
    return &db->header;
}
#endif // ZR_ASSET_DB_CPP
#ifndef ZR_ASSET_DB_CPP
#define ZR_ASSET_DB_CPP

#include "zr_asset_db.h"

#define ZR_ASSET_DB_MAX_HANDLES 512

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

/**
 * Load an entire file, unaltered, into memory.
 * Must be freed by the caller after use
 */
extern "C" i32 ZRDB_StageRawFile(char* path, ZEByteBuffer* dest)
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

#define ZRDB_CAST_TO_INTERNAL(assetDBHeader, newVarName) ZRAssetDBData*##newVarName##=##(ZRAssetDBData*)##assetDBHeader##;

#include "zr_db_textures.h"
#include "zr_db_meshes.h"

///////////////////////////////////////////////////////////////////////////
// Textures
///////////////////////////////////////////////////////////////////////////

static i32 ZRDB_GetMaterialIndexByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = 0;
    for (i32 i = 0; i < db->numMaterials; ++i)
    {
        if (ZE_CompareStrings(name, db->materials[i].name) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

static ZRMaterial* ZRDB_GetFreeMaterial(ZRAssetDB* assetDB, char* newName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	i32 index = db->numMaterials++;
	ZRMaterial* mat = &db->materials[index];
    mat->id = index;
	mat->name = newName;
	return mat;
}

static ErrorCode ZRDB_CreateMaterial(
	ZRAssetDB* assetDB, char* name, char* diffuseName, char* emissiveName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	ZRMaterial* mat = ZRDB_GetFreeMaterial(assetDB, name);
	mat->diffuseTexIndex = ZRDB_GetTexIndexByName(assetDB, diffuseName);
	mat->emissionTexIndex = ZRDB_GetTexIndexByName(assetDB, emissiveName);
    printf("ZRDB Create material id %d: \"%s\"\n", mat->id, mat->name);
    return ZE_ERROR_NONE;
}

static void ZRDB_GetMaterialByIndex(ZRAssetDB* assetDB, i32 index, ZRMaterial* result)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	if (index < 0 || index >= db->numMaterials) { index = 0; }
    *result = db->materials[index];
}

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

    // Get meshes
    //db->header.GetMeshIndexByName = ZRDB_GetMeshIndexByName;
    db->header.GetMeshHandleByName = ZRDB_GetMeshHandlesByName;
    // Get textures
    //db->header.GetTexIndexByName = ZRDB_GetTexIndexByName;
    db->header.GetTextureHandleByName = ZRDB_GetTexHandleByName;
    // Get Materials
    //db->header.GetMaterialIndexByName = ZRDB_GetMaterialIndexByName;
    // Load
    db->header.CreateMaterial = ZRDB_CreateMaterial;
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
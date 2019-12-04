#ifndef ZR_ASSET_DB_CPP
#define ZR_ASSET_DB_CPP

#include "zr_asset_db.h"

#define ZR_ASSET_DB_MAX_HANDLES 512

struct ZRDBTexture
{
    char* fileName;
    void* data;
    i32 dataSize;
    i32 width;
    i32 height;
    i32 apiHandle;
};

struct ZRDBMesh
{
    char* name;
    // api handles
    ZRMeshHandles handles;
    // mesh data on heap
    MeshData data;
};

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

//internal ZRDBTexture g_textures[ZR_ASSET_DB_MAX_HANDLES];
//internal i32 g_nextTexture = 0;

static i32 ZRDB_RegisterTexture(
    ZRAssetDB* assetDB, char* fileName, void* data, i32 dataSize, i32 width, i32 height, i32 apiHandle)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    
    i32 index = db->numTextures++;
    printf("ZRDB - registered texture %d: %s, handle %d\n",
        index, fileName, apiHandle);
    ZRDBTexture* handle = &db->textures[index];
    handle->fileName = fileName;
    handle->data = data;
    handle->dataSize = dataSize;
    handle->width = width;
    handle->height = height;
    handle->apiHandle = apiHandle;
    return index;
}

static i32 ZRDB_GetTexIndexByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    for (i32 i = 0; i < db->numTextures; ++i)
    {
        if (ZE_CompareStrings(name, db->textures[i].fileName) == 0)
        {
            return i;
        }
    }
    return 0;
}

static i32 ZRDB_GetTexHandleByIndex(ZRAssetDB* assetDB, i32 index)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    if (index < 0 || index >= db->numTextures) { return 0; }
    return db->textures[index].apiHandle;
}

static void ZRDB_GetTexHandleByName(ZRAssetDB* assetDB, char* name, i32* result)
{
    i32 index = ZRDB_GetTexIndexByName(assetDB, name);
    *result = ZRDB_GetTexHandleByIndex(assetDB, index);
}


///////////////////////////////////////////////////////////////////////////
// Meshes
///////////////////////////////////////////////////////////////////////////

//internal ZRDBMesh g_meshes[ZR_ASSET_DB_MAX_HANDLES];
//internal i32 g_nextMesh = 0;

static i32 ZRDB_RegisterMesh(ZRAssetDB* assetDB, char* name, ZRMeshHandles handles, MeshData data)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = db->numMeshes++;
    ZRDBMesh* mesh = &db->meshes[index];
    mesh->name = name;
    mesh->handles = handles;
    mesh->data = data;
    printf("ZRDB - registered Mesh %s at index %d\n", name, index);
    return index;
}

static i32 ZRDB_GetMeshIndexByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = 0;
    for (i32 i = 0; i < db->numMeshes; ++i)
    {
        if (ZE_CompareStrings(name, db->meshes[i].name) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

static void ZRDB_GetMeshHandlesByIndex(ZRAssetDB* assetDB, i32 index, ZRMeshHandles* result)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    if (index < 0 || index >= db->numMeshes) { index = 0; }
    *result = db->meshes[index].handles;
}

static void ZRDB_GetMeshHandlesByName(ZRAssetDB* assetDB, char* name, ZRMeshHandles* result)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = ZRDB_GetMeshIndexByName(assetDB, name);
    ZRDB_GetMeshHandlesByIndex(assetDB, index, result);
}

///////////////////////////////////////////////////////////////////////////
// Materials
///////////////////////////////////////////////////////////////////////////

//internal ZRMaterial g_materials[ZR_ASSET_DB_MAX_HANDLES];
//internal i32 g_nextMaterial = 0;

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
    db->header.GetMaterialIndexByName = ZRDB_GetMaterialIndexByName;
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
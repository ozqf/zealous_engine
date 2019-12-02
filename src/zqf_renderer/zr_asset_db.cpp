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
// Textures
///////////////////////////////////////////////////////////////////////////

internal ZRDBTexture g_textures[ZR_ASSET_DB_MAX_HANDLES];
internal i32 g_nextTexture = 0;

extern "C" i32 ZRDB_RegisterTexture(
    char* fileName, void* data, i32 dataSize, i32 width, i32 height, i32 apiHandle)
{
    i32 index = g_nextTexture++;
    printf("ZRDB - registered texture %d: %s, handle %d\n",
        index, fileName, apiHandle);
    ZRDBTexture* handle = &g_textures[index];
    handle->fileName = fileName;
    handle->data = data;
    handle->dataSize = dataSize;
    handle->width = width;
    handle->height = height;
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

extern "C" i32 ZRDB_GetTexHandleByName(char* name)
{
    i32 index = ZRDB_GetTexIndexByName(name);
    return ZRDB_GetTexHandleByIndex(index);
}


///////////////////////////////////////////////////////////////////////////
// Meshes
///////////////////////////////////////////////////////////////////////////

internal ZRDBMesh g_meshes[ZR_ASSET_DB_MAX_HANDLES];
internal i32 g_nextMesh = 0;

extern "C" i32 ZRDB_RegisterMesh(char* name, ZRMeshHandles handles, MeshData data)
{
    i32 index = g_nextMesh++;
    ZRDBMesh* mesh = &g_meshes[index];
    mesh->name = name;
    mesh->handles = handles;
    mesh->data = data;
    printf("ZRDB - registered Mesh %s at index %d\n", name, index);
    return index;
}

extern "C" i32 ZRDB_GetMeshIndexByName(char* name)
{
    i32 index = 0;
    for (i32 i = 0; i < g_nextMesh; ++i)
    {
        if (ZE_CompareStrings(name, g_meshes[i].name) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

extern "C" void ZRDB_GetMeshHandlesByIndex(i32 index, ZRMeshHandles* result)
{
    if (index < 0 || index >= g_nextMesh) { index = 0; }
    *result = g_meshes[index].handles;
}

extern "C" void ZRDB_GetMeshHandlesByName(char* name, ZRMeshHandles* result)
{
    i32 index = ZRDB_GetMeshIndexByName(name);
    ZRDB_GetMeshHandlesByIndex(index, result);
}

///////////////////////////////////////////////////////////////////////////
// Materials
///////////////////////////////////////////////////////////////////////////

internal ZRMaterial g_materials[ZR_ASSET_DB_MAX_HANDLES];
internal i32 g_nextMaterial = 0;

extern "C" i32 ZRDB_GetMaterialIndexByName(char* name)
{
    i32 index = 0;
    for (i32 i = 0; i < g_nextMesh; ++i)
    {
        if (ZE_CompareStrings(name, g_materials[i].name) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

static ZRMaterial* ZRDB_GetFreeMaterial(char* newName)
{
	i32 index = g_nextMaterial++;
	ZRMaterial* mat = &g_materials[index];
    mat->id = index;
	mat->name = newName;
	return mat;
}

extern "C" void ZRDB_CreateMaterial(
	char* name, char* diffuseName, char* emissiveName)
{
	ZRMaterial* mat = ZRDB_GetFreeMaterial(name);
	mat->diffuseTexIndex = ZRDB_GetTexIndexByName(diffuseName);
	mat->emissionTexIndex = ZRDB_GetTexIndexByName(emissiveName);
    printf("ZRDB Create material id %d: \"%s\"\n", mat->id, mat->name);
}

extern "C" void ZRDB_GetMaterialByIndex(i32 index, ZRMaterial* result)
{
	if (index < 0 || index >= g_nextMaterial) { index = 0; }
    *result = g_materials[index];
}

///////////////////////////////////////////////////////////////////////////
// Create
///////////////////////////////////////////////////////////////////////////

extern "C" ZRAssetDB* ZRDB_Create()
{
    ZRAssetDBData* db = (ZRAssetDBData*)malloc(sizeof(ZRAssetDBData));
    *db = {};
    //db->header.GetMaterialIndexByName = ZRDB_GetMaterialIndexByName;
    //db->header.GetMeshIndexByName = ZRDB_GetMeshIndexByName;
    //db->header.GetTexIndexByName = ZRDB_GetTexIndexByName;

    db->textures = (ZRDBTexture*)malloc(sizeof(ZRDBTexture) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxTextures = ZR_ASSET_DB_MAX_HANDLES;
    db->meshes = (ZRDBMesh*)malloc(sizeof(ZRDBMesh) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMeshes = ZR_ASSET_DB_MAX_HANDLES;
    db->materials = (ZRMaterial*)malloc(sizeof(ZRMaterial) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMaterials = ZR_ASSET_DB_MAX_HANDLES;
    return &db->header;
}
#endif // ZR_ASSET_DB_CPP
#ifndef ZR_DB_MESHES_H
#define ZR_DB_MESHES_H

#include "zr_asset_db.h"

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

static i32 ZRDB_LoadMesh(ZRAssetDB* assetDB, char* name, MeshData* data, i32 bVerbose)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = db->numMeshes++;
    ZRDBMesh* mesh = &db->meshes[index];
    mesh->name = name;
    mesh->data = *data;
    db->uploader.UploadMesh(data, &mesh->handles, 0);
    printf("ZRDB - registered mesh %s handle %d\n", name, mesh->handles.vao);
    return 0;
}

static i32 ZRDB_LoadMeshFromFBX(ZRAssetDB* assetDB, char* path, Vec3 reScale, i32 bSwapYZ, i32 bVerbose)
{
    return 0;
}

#endif // ZR_DB_MESHES_H
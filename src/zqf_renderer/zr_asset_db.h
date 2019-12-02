#ifndef ZR_ASSET_DB_H
#define ZR_ASSET_DB_H

#include "../zqf_renderer.h"

#define ZRDB_ASSET_TYPE_NONE 0
#define ZRDB_ASSET_TYPE_TEXTURE 1
#define ZRDB_ASSET_TYPE_ 2
#define ZRDB_ASSET_TYPE_BLOB 3

struct ZRAssetDB
{
    i32 (*GetTexIndexByName)(ZRAssetDB* db, char* name);
    i32 (*GetMaterialIndexByName)(ZRAssetDB* db, char* name);
    i32 (*GetMeshIndexByName)(ZRAssetDB* db, char* name);
};

/**
 * Ultra simple way to store assets and retrieve by name or opengl handle
 */
extern "C" i32 ZRDB_RegisterTexture(
    char* fileName, void* data, i32 dataSize, i32 width, i32 height, i32 apiHandle);
extern "C" i32 ZRDB_GetTexIndexByName(char* name);
extern "C" i32 ZRDB_GetTexHandleByIndex(i32 index);
extern "C" i32 ZRDB_GetTexHandleByName(char* name);

extern "C" i32 ZRDB_RegisterMesh(char* name, ZRMeshHandles handles, MeshData data);
extern "C" i32 ZRDB_GetMeshIndexByName(char* name);
extern "C" void ZRDB_GetMeshHandlesByIndex(i32 index, ZRMeshHandles* result);
extern "C" void ZRDB_GetMeshHandlesByName(char* name, ZRMeshHandles* result);

extern "C" void ZRDB_CreateMaterial(char* name, char* diffuseName, char* emissiveName);
extern "C" i32 ZRDB_GetMaterialIndexByName(char* name);
extern "C" void ZRDB_GetMaterialByIndex(i32 index, ZRMaterial* result);

extern "C" ZRAssetDB* ZRDB_Create();
#endif // ZR_ASSET_DB_H
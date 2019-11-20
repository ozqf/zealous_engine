#ifndef ZR_ASSET_DB_H
#define ZR_ASSET_DB_H

#include "../zqf_renderer.h"

#define ZRDB_ASSET_TYPE_NONE 0
#define ZRDB_ASSET_TYPE_TEXTURE 1
#define ZRDB_ASSET_TYPE_ 2
#define ZRDB_ASSET_TYPE_BLOB 3

/**
 * Ultra simple way to store assets and retrieve by name or opengl handle
 */
extern "C" i32 ZRDB_RegisterTexture(char* fileName, void* data, i32 dataSize, i32 apiHandle);
extern "C" i32 ZRDB_GetTexIndexByName(char* name);
extern "C" i32 ZRDB_GetTexHandleByIndex(i32 index);

//extern "C" i32 ZRDB_RegisterMesh(char* fileName, void* data, i32 dataSize, i32 apiHandle);
//extern "C" i32 ZRDB_GetMeshIndexByName(char* name);
//extern "C" i32 ZRDB_GetMeshHandleByIndex(i32 index);

#endif // ZR_ASSET_DB_H
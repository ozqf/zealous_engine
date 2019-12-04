#ifndef ZR_ASSET_DB_H
#define ZR_ASSET_DB_H

#include "../zqf_renderer.h"

#define ZRDB_ASSET_TYPE_NONE 0
#define ZRDB_ASSET_TYPE_TEXTURE 1
#define ZRDB_ASSET_TYPE_ 2
#define ZRDB_ASSET_TYPE_BLOB 3

///////////////////////////////////////////////////////////
// Asset data types
///////////////////////////////////////////////////////////
struct ZRDBTexture
{
    char* fileName;
    void* data;
    i32 dataSize;
    i32 width;
    i32 height;
    i32 apiHandle;
};

struct MeshData
{
	u32 numVerts;

	f32* verts;
	f32* uvs;
    f32* normals;    
};

// internal types
/**
 * Asset handles required to execute a draw call
 */
struct ZRMeshHandles
{
    i32 vao;
    i32 vbo;
    i32 vertexCount;
	i32 totalVBOBytes;
	// all data before this point is static mesh geometry
	i32 instanceDataOffset;
	// Capacity for instances left behind static mesh data
	i32 maxInstances;
};

struct ZRDBMesh
{
    char* name;
    // api handles
    ZRMeshHandles handles;
    // mesh data on heap
    MeshData data;
};

struct ZRAssetDB
{
    //i32 (*GetTexIndexByName)(ZRAssetDB* assetDB, char* name);
    //i32 (*GetMaterialIndexByName)(ZRAssetDB* assetDB, char* name);
    //i32 (*GetMeshIndexByName)(ZRAssetDB* assetDB, char* name);

    void (*GetMeshHandleByName)(ZRAssetDB* assetDB, char* name, ZRMeshHandles* result);
    void (*GetTextureHandleByName)(ZRAssetDB* assetDB, char* name, i32* result);

    ErrorCode (*CreateMaterial)(ZRAssetDB* assetDB, char* name, char* diffuseTexName, char* emissiveTexName);

    i32 (*LoadTexture)(ZRAssetDB* assetDB, char* path, i32 bVerbose);
    i32 (*LoadMeshFromFBX)(ZRAssetDB* assetDB, char* path, Vec3 reScale, i32 bSwapYZ, i32 bVerbose);
    i32 (*LoadMesh)(ZRAssetDB* assetDB, char* name, MeshData* data, i32 bVerbose);
};

struct ZRAssetUploader
{
    void (*UploadTexture)(u8* pixels, i32 width, i32 height, u32* handle);
    void (*UploadMesh)(MeshData* data, ZRMeshHandles* result, u32 flags);
};

/**
 * Ultra simple way to store assets and retrieve by name or opengl handle
 */
/*
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
*/
extern "C" ZRAssetDB* ZRDB_Create(ZRAssetUploader uploader);
#endif // ZR_ASSET_DB_H
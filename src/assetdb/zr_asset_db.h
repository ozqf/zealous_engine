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

struct ZRAsset
{
    i32 id;
    i32 index;
    i32 type;
    char* fileName;
};

struct ZRDBTexture
{
    //char* fileName;
    ZRAsset header;
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
	i32 index;
    char* name;
    // api handles
    ZRMeshHandles handles;
    // mesh data on heap
    MeshData data;
};

struct ZRAssetDB
{
    ZRDBMesh* (*GetMeshByName)(ZRAssetDB* assetDB, char* name);
    ZRDBMesh* (*GetMeshByIndex)(ZRAssetDB* assetDB, i32 index);
    void (*GetMeshHandleByName)(ZRAssetDB* assetDB, char* name, ZRMeshHandles* result);

    ZRDBTexture* (*GetTextureByName)(ZRAssetDB* assetDB, char* name);
    i32 (*GetTextureHandleByIndex)(ZRAssetDB* assetDB, i32 index);

    void (*CreateMaterial)(ZRAssetDB* assetDB, char* name, char* diffuseTexName, char* emissiveTexName);
    ZRMaterial* (*GetMaterialByName)(ZRAssetDB* assetDB, char* name);
    ZRMaterial* (*GetMaterialByIndex)(ZRAssetDB* assetDB, i32 index);

    i32 (*LoadTexture)(ZRAssetDB* assetDB, char* path, i32 bVerbose);
    i32 (*LoadMeshFromFBX)(ZRAssetDB* assetDB, char* path, Vec3 reScale, i32 bSwapYZ, i32 bVerbose);
    i32 (*LoadMesh)(ZRAssetDB* assetDB, char* name, MeshData* data, i32 bVerbose);
};

struct ZRAssetUploader
{
    void (*UploadTexture)(u8* pixels, i32 width, i32 height, u32* handle);
    void (*UploadMesh)(MeshData* data, ZRMeshHandles* result, u32 flags);
};

extern "C" ZRAssetDB* ZRDB_Create();
extern "C" void ZRDB_AttachUploader(ZRAssetDB* assetDB, ZRAssetUploader uploader);
extern "C" void ZRDB_PrintManifest(ZRAssetDB* assetDB);
#endif // ZR_ASSET_DB_H
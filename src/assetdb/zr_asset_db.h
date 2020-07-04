#ifndef ZR_ASSET_DB_H
#define ZR_ASSET_DB_H

#include "../zqf_renderer.h"

#define ZRDB_ASSET_TYPE_NONE 0
#define ZRDB_ASSET_TYPE_TEXTURE 1
#define ZRDB_ASSET_TYPE_ 2
#define ZRDB_ASSET_TYPE_BLOB 3

#define ZRDB_MESH_NAME_CUBE "Cube"
#define ZRDB_MESH_NAME_INVERSE_CUBE "InverseCube"
#define ZRDB_MESH_NAME_QUAD "Quad"
#define ZRDB_MESH_NAME_DYNAMIC_QUAD "DynamicQuad"
#define ZRDB_MESH_NAME_SPIKE "Spike"

#define ZQF_R_DEFAULT_DIFFUSE_TEX "data/W33_5.bmp"
#define ZR_TRANSPARENT_TEX_NAME "transparent"

#define ZRDB_DEFAULT_DIFFUSE_MAT_NAME "default"
#define ZRDB_DEFAULT_CHARSET_MAT_NAME "charset"
#define ZRDB_DEFAULT_CHARSET_NAME "charset"

#define ZRDB_MAT_NAME_WORLD "World"
#define ZRDB_MAT_NAME_ENT "Entity"
#define ZRDB_MAT_NAME_ENEMY "Enemy"
#define ZRDB_MAT_NAME_PRJ "Projectile"
#define ZRDB_MAT_NAME_GFX "Gfx"
#define ZRDB_MAT_NAME_LASER "Laser"
#define ZRDB_MAT_NAME_LIGHT "Light"
#define ZRDB_MAT_NAME_WORLD_DEBUG "WorldDebug"

///////////////////////////////////////////////////////////
// Asset data types
///////////////////////////////////////////////////////////

struct ZRAsset
{
    i32 id;
    i32 index;
    i32 type;
	i32 bIsUploaded;
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
	ZRAsset header;
    // api handles
    ZRMeshHandles handles;
    // mesh data on heap
    MeshData data;
};

/**
 * Architecture of this is... a bit wild?
 */
struct ZRAssetDB
{
    ZRDBMesh* (*GetMeshByName)(ZRAssetDB* assetDB, char* name);
    ZRDBMesh* (*GetMeshByIndex)(ZRAssetDB* assetDB, i32 index);
    void (*GetMeshHandleByName)(ZRAssetDB* assetDB, char* name, ZRMeshHandles* result);
    i32 (*GetNumMeshes)(ZRAssetDB* assetDB);

    ZRDBTexture* (*GetTextureByName)(ZRAssetDB* assetDB, char* name);
    ZRDBTexture* (*GetTextureByIndex)(ZRAssetDB* assetDB, i32 index);
    i32 (*GetTextureHandleByIndex)(ZRAssetDB* assetDB, i32 index);
    i32 (*GetNumTextures)(ZRAssetDB* assetDB);

    ZRMaterial* (*CreateMaterial)(ZRAssetDB* assetDB, char* name, char* diffuseTexName, char* emissiveTexName);
    ZRMaterial* (*GetMaterialByName)(ZRAssetDB* assetDB, char* name);
    ZRMaterial* (*GetMaterialByIndex)(ZRAssetDB* assetDB, i32 index);
    i32 (*GetNumMaterials)(ZRAssetDB* assetDB);

    i32 (*LoadTexture)(ZRAssetDB* assetDB, char* path, i32 bVerbose);
    i32 (*LoadMeshFromFBX)(ZRAssetDB* assetDB, char* path, Vec3 reScale, i32 bSwapYZ, i32 bVerbose);
    i32 (*LoadMesh)(ZRAssetDB* assetDB, char* name, MeshData* data, i32 bVerbose);

    void (*VidRestart)(ZRAssetDB* assetDB);
};

struct ZRAssetUploader
{
    void (*UploadTexture)(u8* pixels, i32 width, i32 height, u32* handle);
    void (*UploadMesh)(MeshData* data, ZRMeshHandles* result, u32 flags);
};

#define ZRDB_GET_MAT_BY_NAME(assetDbPtr, matNameChars) \
assetDbPtr->GetMaterialByName(##assetDbPtr##, ##matNameChars##)

#define ZRDB_GET_MESH_BY_NAME(assetDbPtr, matNameChars) \
assetDbPtr->GetMeshByName(##assetDbPtr##, ##matNameChars##)

extern "C" ZRAssetDB* ZRDB_Create();
extern "C" void ZRDB_AttachUploader(ZRAssetDB* assetDB, ZRAssetUploader uploader);
extern "C" void ZRDB_PrintManifest(ZRAssetDB* assetDB);
#endif // ZR_ASSET_DB_H
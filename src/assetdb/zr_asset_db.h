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
#define ZRDB_TEX_NAME_CROSSHAIR "crosshair"
#define ZRDB_TEX_NAME_SHEET_TEST "sheet_test"

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
#define ZRDB_MAT_NAME_CROSSHAIR "crosshair"

///////////////////////////////////////////////////////////
// Asset data types
///////////////////////////////////////////////////////////

struct ZRAsset
{
    i32 id;
    i32 index;
    i32 type;
	// On GPU - handles are set
	i32 bIsUploaded;
	// Data has changed - needs to be re-uploaded
	i32 bIsDirty;
    char* fileName;
};

struct ZRDBBlob
{
    ZRAsset header;
	ZEBuffer data;
};

struct ZRDBTexture
{
    //char* fileName;
    ZRAsset header;
	// 32 bit pixel data.
    ColourU32* data;
    i32 dataSize;
    i32 width;
    i32 height;
    i32 apiHandle;
};

#define VEC3_SIZE = 12
#define VEC2_SIZE = 8

// Currently stores every vertex, no sharing
struct MeshData
{
	u32 numVerts;
	// dynamic mesh may have more capacity.
	u32 maxVerts;

	f32* verts;
	f32* uvs;
    f32* normals;

	Vec3* GetVert(i32 i) { return (Vec3*)(verts + (i * 3)); }

	i32 MeasureBytes()
	{
		i32 bytes = 0;
		const i32 v3size = sizeof(f32) * 3;
		const i32 v2size = sizeof(f32) * 2;
		bytes += v3size * numVerts;
		bytes += v2size * numVerts;
		bytes += v3size * numVerts;
		return bytes;
	}

	i32 CopyData(MeshData original)
	{
		if (original.numVerts > maxVerts)
		{
			printf("No space to copy mesh! %d verts have %d\n",
				original.numVerts, maxVerts);
			return ZE_ERROR_NO_SPACE;
		}
		numVerts = original.numVerts;
		const i32 numVertBytes = (sizeof(f32) * 3) * numVerts;
		const i32 numUVSBytes = (sizeof(f32) * 2) * numVerts;
		printf("Copying %d verts (%d vert bytes, %d uv bytes)\n",
			numVerts, numVertBytes, numUVSBytes);
		ZE_Copy(verts, original.verts, numVertBytes);
		ZE_Copy(uvs, original.uvs, numUVSBytes);
		ZE_Copy(normals, original.normals, numVertBytes);
		return ZE_ERROR_NONE;
	}

	void PrintVerts()
	{
		printf("--- %d verts ---\n", numVerts);
		f32* cursor = verts;
		for (u32 i = 0; i < numVerts; ++i)
		{
			printf("%d: %.3f, %.3f, %.3f\n", i, cursor[0], cursor[1], cursor[2]);
			cursor += 3;
		}
	}
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

struct ZRDBMaterial
{
	ZRAsset header;
	ZRMaterial data;
};

/**
 * Architecture of this is... a bit wild?
 */
struct ZRAssetDB
{
	// Fields
	volatile i32 bDirty;
	
	// "Methods"
    ZRDBMesh* (*GetMeshByName)(ZRAssetDB* assetDB, char* name);
    ZRDBMesh* (*GetMeshByIndex)(ZRAssetDB* assetDB, i32 index);
    void (*GetMeshHandleByName)(ZRAssetDB* assetDB, char* name, ZRMeshHandles* result);
    i32 (*GetNumMeshes)(ZRAssetDB* assetDB);

	i32 (*ZRDB_GetBlobIndexName)(ZRAssetDB* handle, char* name);
	ZRDBBlob* (*ZRDB_GetBlobByName)(ZRAssetDB* assetDB, char* name);
	ZRDBBlob* (*ZRDB_CreateBlob)(ZRAssetDB* assetDB, char* name, i32 numBytes);

    ZRDBTexture* (*GetTextureByName)(ZRAssetDB* assetDB, char* name);
    ZRDBTexture* (*GetTextureByIndex)(ZRAssetDB* assetDB, i32 index);
    i32 (*GetTextureHandleByIndex)(ZRAssetDB* assetDB, i32 index);
    i32 (*GetNumTextures)(ZRAssetDB* assetDB);
	ZRDBTexture* (*GenBlankTexture)(ZRAssetDB* handle, char* name, i32 w, i32 h, ColourU32 fill);

    ZRDBMaterial* (*CreateMaterial)(ZRAssetDB* assetDB, char* name, char* diffuseTexName, char* emissiveTexName);
    ZRDBMaterial* (*GetMaterialByName)(ZRAssetDB* assetDB, char* name);
    ZRDBMaterial* (*GetMaterialByIndex)(ZRAssetDB* assetDB, i32 index);
    i32 (*GetNumMaterials)(ZRAssetDB* assetDB);

    i32 (*LoadTexture)(ZRAssetDB* assetDB, char* path, i32 bVerbose);
    i32 (*LoadMeshFromFile)(ZRAssetDB* assetDB, char* path, Vec3 reScale, i32 bSwapYZ, i32 bVerbose);
    ZRDBMesh* (*LoadMesh)(ZRAssetDB* assetDB, char* name, const MeshData data, i32 bVerbose);
	ZRDBMesh* (*CreateEmptyMesh)(ZRAssetDB* assetDB, char* name, i32 maxVerts);

    void (*VidRestart)(ZRAssetDB* assetDB);
};
#if 0
struct ZRAssetUploader
{
    void (*UploadTexture)(u8* pixels, i32 width, i32 height, u32* handle);
    void (*UploadMesh)(MeshData* data, ZRMeshHandles* result, u32 flags);
};
#endif
#define ZRDB_GET_MAT_BY_NAME(assetDbPtr, matNameChars) \
assetDbPtr->GetMaterialByName(##assetDbPtr##, ##matNameChars##)

#define ZRDB_GET_MESH_BY_NAME(assetDbPtr, matNameChars) \
assetDbPtr->GetMeshByName(##assetDbPtr##, ##matNameChars##)

extern "C" ZRAssetDB* ZRDB_Create(ZE_FatalErrorFunction errorHandler, ZEFileIO files);
//extern "C" void ZRDB_AttachUploader(ZRAssetDB* assetDB, ZRAssetUploader uploader);
extern "C" void ZRDB_PrintManifest(ZRAssetDB* assetDB);
#endif // ZR_ASSET_DB_H
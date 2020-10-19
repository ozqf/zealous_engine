#ifndef ZR_DB_MESHES_H
#define ZR_DB_MESHES_H

#include "zr_asset_db.h"

static i32 ZRDB_GetNumMeshes(ZRAssetDB* assetDB)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    return db->numMeshes;
}

static i32 ZRDB_RegisterMesh(ZRAssetDB* assetDB, char* name, ZRMeshHandles handles, MeshData data)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = db->numMeshes++;
    ZRDBMesh* mesh = &db->meshes[index];
    *mesh = {};
    mesh->header.fileName = name;
    mesh->header.id = db->nextId;
    db->nextId++;
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
        if (ZStr_Compare(name, db->meshes[i].header.fileName) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

static ZRDBMesh* ZRDB_GetMeshByIndex(ZRAssetDB* assetDB, i32 index)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    if (index < 0 || index >= db->numMeshes) { index = 0; }
    return &db->meshes[index];
}

static ZRDBMesh* ZRDB_GetMeshByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = ZRDB_GetMeshIndexByName(assetDB, name);
    return ZRDB_GetMeshByIndex(assetDB, index);
}

static void ZRDB_GetMeshHandleByName(ZRAssetDB* assetDB, char* name, ZRMeshHandles* result)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = ZRDB_GetMeshIndexByName(assetDB, name);
    *result = db->meshes[index].handles;
}

static MeshData ZRDB_AllocateMeshData(i32 maxVerts)
{
	MeshData clone = {};
	clone.numVerts = 0;
	clone.maxVerts = maxVerts;
	i32 numVertBytes = (sizeof(f32) * 3) * maxVerts;
	i32 numUVBytes = (sizeof(f32) * 2) * maxVerts;
	i32 totalBytes = numVertBytes + numUVBytes + numVertBytes;
	u8* bytes = (u8*)malloc(totalBytes);
    ZE_SET_ZERO(bytes, totalBytes)
	clone.verts = (f32*)bytes;
	clone.uvs = (f32*)(bytes + numVertBytes);
	clone.normals = (f32*)(bytes + numVertBytes + numUVBytes);
	return clone;
}

static MeshData ZRDB_CopyMeshData(const MeshData original)
{
	MeshData clone = {};
	clone.numVerts = original.numVerts;
	i32 numVertBytes = (sizeof(f32) * 3) * original.numVerts;
	i32 numUVBytes = (sizeof(f32) * 2) * original.numVerts;
	i32 totalBytes = numVertBytes + numUVBytes + numVertBytes;

	u8* bytes = (u8*)malloc(totalBytes);
    ZE_SET_ZERO(bytes, totalBytes)
	// copy verts
	clone.verts = (f32*)bytes;
	bytes += ZE_Copy(bytes, original.verts, numVertBytes);
	clone.uvs = (f32*)bytes;
	bytes += ZE_Copy(bytes, original.uvs, numUVBytes);
	clone.normals = (f32*)bytes;
	bytes += ZE_Copy(bytes, original.normals, numVertBytes);
	return clone;
}

static ZRDBMesh* ZRDB_AddMesh(ZRAssetDB* assetDB, char* name, MeshData data, i32 bVerbose)
{
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = db->numMeshes++;
    ZRDBMesh* mesh = &db->meshes[index];
	mesh->header.index = index;
    mesh->header.fileName = name;
    mesh->data = data;
    return mesh;
}

/**
 * Allocate space for an empty mesh with the given vertex capacity
 * create a mesh entry and return it
 */
static ZRDBMesh* ZRDB_CreateEmptyMesh(ZRAssetDB* assetDB, char* name, i32 maxVerts)
{
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
	MeshData data = ZRDB_AllocateMeshData(maxVerts);
	ZRDBMesh* mesh = NULL;
	mesh = ZRDB_AddMesh(assetDB, name, data, NO);
    printf("Created empty mesh %s with %d max verts\n", name, maxVerts);
	return mesh;
}

/**
 * Takes mesh data, clones it and creates a new mesh entry
 * for when mesh data comes from an external source like a file
 */
static ZRDBMesh* ZRDB_LoadMeshFromData(
    ZRAssetDB* assetDB, char* name, const MeshData data, i32 bVerbose)
{
	const MeshData clone = ZRDB_CopyMeshData(data);
	ZRDBMesh* mesh = ZRDB_AddMesh(assetDB, name, clone, bVerbose);
	return mesh;
}

static i32 ZRDB_LoadMeshFromFile(
    ZRAssetDB* assetDB, char* path, Vec3 reScale, i32 bSwapYZ, i32 bVerbose)
{
    i32 pathLen = ZStr_Len(path);
    if (ZStr_CheckExtension(path, pathLen, ".obj"))
    {
        
    }
    else if (ZStr_CheckExtension(path, pathLen, ".fbx"))
    {
        ILLEGAL_CODE_PATH
        // TODO resurrect old FBX load code in zr_db_fbx.h
        MeshData data;
		i32 err = ZRGL_LoadFBX(path, reScale, bSwapYZ, YES, &data);
		if (err == ZE_ERROR_NONE)
		{
			ZRDB_AddMesh(assetDB, path, data, NO);
		}
    }
    else
    {
        ZE_ASSERT(0, "Unknown model file extension");
    }

    return 0;
}

#endif // ZR_DB_MESHES_H
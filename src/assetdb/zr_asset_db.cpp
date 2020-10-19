#ifndef ZR_ASSET_DB_CPP
#define ZR_ASSET_DB_CPP

#include "zr_db_internal.h"

///////////////////////////////////////////////////////////////////////////
// Implementations
///////////////////////////////////////////////////////////////////////////

#include "zr_db_textures.h"
#include "zr_db_meshes.h"
#include "zr_db_materials.h"
#include "zr_db_fbx.h"
#include "zrdb_gen.h"

///////////////////////////////////////////////////////////////////////////
// Print loaded asset list
///////////////////////////////////////////////////////////////////////////
extern "C" void ZRDB_PrintManifest(ZRAssetDB* assetDB)
{
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
	printf("=== Asset Manifest ===\n");
	printf("--- Meshes (%d) ---\n", assetDB->GetNumMeshes(assetDB));
	for (i32 i = 0; i < db->numMeshes; ++i)
	{
		ZRDBMesh* mesh = &db->meshes[i];
		printf("%d: %s\n", i, mesh->header.fileName);
	}
	printf("--- Textures (%d) --- \n", assetDB->GetNumTextures(assetDB));
	for (i32 i = 0; i < db->numTextures; ++i)
	{
		ZRDBTexture* tex = &db->textures[i];
		printf("%d: %s\n", i, tex->header.fileName);
	}
	printf("--- Materials (%d) ---\n", db->numMaterials);
	for (i32 i = 0; i < db->numMaterials; ++i)
	{
		ZRDBMaterial* mat = &db->materials[i];
		printf("%d: %s: diffuse: %d\n", i,
			mat->header.fileName, mat->data.diffuseTexIndex);
	}
	
	printf("--- Allocations (%d of %d - %dMB)\n",
		db->allocs.next, db->allocs.max, COM_SumMallocs(&db->allocs) / (1024 * 1024));
	for (i32 i = 0; i < db->allocs.next; ++i)
	{
		printf("%d: %dKB - %s\n",
			i, db->allocs.items[i].capacity, db->allocs.items[i].label);
	}
	printf("Strings %d of %d bytes\n", db->strings.Written(), db->strings.capacity);
	printf("\n");
}

internal void ZRDB_VidRestart(ZRAssetDB* assetDb)
{
	// Clear all GPU upload handles
	// tbh... mapping asset to gpu should be handled by the renderer in a
	// hash table or something... asset db shouldn't care.

	ZRDB_CAST_TO_INTERNAL(assetDb, db)
	printf("ZRDB - Clearing ALL GPU handles\n");
	for (i32 i = 0; i < db->numTextures; ++i)
	{
		db->textures[i].header.bIsUploaded = NO;
		db->textures[i].apiHandle = 0;
	}
	for (i32 i = 0; i < db->numMeshes; ++i)
	{
		db->meshes[i].header.bIsUploaded = NO;
		db->meshes[i].handles = {};
	}
	// currently materials store asset db indices
	// and are looked up at render time... so don't need to clear.
	// for (i32 i = 0; i < db->numMaterials; ++i)
	// {
		
	// }
}

///////////////////////////////////////////////////////////////////////////
// Create
///////////////////////////////////////////////////////////////////////////

extern "C" ZRAssetDB* ZRDB_Create(ZE_FatalErrorFunction errorHandler)
{
	ZE_SetFatalError(errorHandler);
	const i32 maxTrackedAllocs = 1024;
	i32 spaceForStruct = sizeof(ZRAssetDBData);
	i32 spaceForAllocList = sizeof(MallocItem) * maxTrackedAllocs;
	i32 spaceForStrings = MegaBytes(1);
	i32 total = spaceForStruct + spaceForAllocList + spaceForStrings;
	u8* ptr = (u8*)malloc(total);
	ZE_SET_ZERO(ptr, total)
	ZRAssetDBData* db = (ZRAssetDBData*)ptr;
	*db = {};

	void* mallocItems = ptr + spaceForStruct;
	db->allocs = COM_InitMallocList((MallocItem*)mallocItems, maxTrackedAllocs);

	// strings space
	db->strings = Buf_FromBytes(ptr + spaceForStruct + spaceForAllocList, spaceForStrings);

	char* strPtr = (char*)db->strings.cursor;
	db->strings.cursor += ZE_CopyStringLimited("ZRDB", strPtr, db->strings.Space());
	COM_SetAlloc(&db->allocs, ptr, total, strPtr);

	printf("ZRDB - %d alloc items %dKB for strings\n", db->allocs.max, db->strings.capacity / 1024);

	// Allocate struct
    //ZRAssetDBData* db = (ZRAssetDBData*)malloc(sizeof(ZRAssetDBData));
    //*db = {};
	// init malloc list

	// allocate space to store strings


    /////////////////////////////////////////////////////
    // functions
    db->nextId = ZR_ASSET_DB_FIRST_ID;

    // Meshes
    db->header.GetMeshByName = ZRDB_GetMeshByName;
    db->header.GetMeshHandleByName = ZRDB_GetMeshHandleByName;
    db->header.GetMeshByIndex = ZRDB_GetMeshByIndex;
    db->header.GetNumMeshes = ZRDB_GetNumMeshes;

	// Textures
    db->header.GetTextureByName = ZRDB_GetTextureByName;
    db->header.GetTextureHandleByIndex = ZRDB_GetTextureHandleByIndex;
    db->header.GetNumTextures = ZRDB_GetNumTextures;
    db->header.GetTextureByIndex = ZRDB_GetTextureByIndex;
	db->header.GenBlankTexture = ZRDB_GenBlankTexture;

	// Materials
    db->header.GetMaterialByName = ZRDB_GetMaterialByName;
    db->header.GetMaterialByIndex = ZRDB_GetMaterialByIndex;
	db->header.GetNumMaterials = ZRDB_GetNumMaterials;
    db->header.CreateMaterial = ZRDB_CreateMaterial;

    // Load
    db->header.LoadMesh = ZRDB_LoadMeshFromData;
    db->header.LoadMeshFromFile = ZRDB_LoadMeshFromFile;
    db->header.LoadTexture = ZRDB_LoadTexture;
	db->header.CreateEmptyMesh = ZRDB_CreateEmptyMesh;

	db->header.VidRestart = ZRDB_VidRestart;

    // store handles to asset data
    db->textures = (ZRDBTexture*)malloc(sizeof(ZRDBTexture) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxTextures = ZR_ASSET_DB_MAX_HANDLES;
    db->meshes = (ZRDBMesh*)malloc(sizeof(ZRDBMesh) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMeshes = ZR_ASSET_DB_MAX_HANDLES;
    db->materials = (ZRDBMaterial*)malloc(sizeof(ZRDBMaterial) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMaterials = ZR_ASSET_DB_MAX_HANDLES;

	ZRDB_LoadEmbedded(&db->header);

    return &db->header;
}
#if 0
extern "C" void ZRDB_AttachUploader(ZRAssetDB* assetDB, ZRAssetUploader uploader)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    db->uploader = uploader;
}
#endif
#endif // ZR_ASSET_DB_CPP
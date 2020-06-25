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
#include "../zr_embedded/zr_embedded.h"

///////////////////////////////////////////////////////////////////////////
// Load embedded assets
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
}

///////////////////////////////////////////////////////////////////////////
// Load embedded assets
///////////////////////////////////////////////////////////////////////////

/**
 * Load in embedded assets. these assets are also default fallbacks.
 */
static void ZRDB_LoadEmbedded(ZRAssetDB* db)
{
    i32 bVerbose = NO;
	MeshData* d;

	d = ZR_Embed_Cube();
    db->LoadMesh(db, ZRDB_MESH_NAME_CUBE, d, YES);
    d = ZR_Embed_InverseCube();
    db->LoadMesh(db, ZRDB_MESH_NAME_INVERSE_CUBE, d, YES);
	d = ZR_Embed_Quad();
    db->LoadMesh(db, ZRDB_MESH_NAME_QUAD, d, YES);
	db->LoadMesh(db, ZRDB_MESH_NAME_DYNAMIC_QUAD, d, YES);
	d = ZR_Embed_Spike();
    db->LoadMesh(db, ZRDB_MESH_NAME_SPIKE, d, YES);

    char* textures[] = {
        "data/W33_5.bmp",
        "data/charset.bmp",
        "data/debug_white.png",
        "data/debug_black.png",
        "data/debug_blue.png",
        "data/debug_red.png",
        "data/debug_red_dark.png",
        "data/debug_yellow.png"
    };
    i32 numTextures = sizeof(textures) / sizeof(char*);
    printf("ZRDB - %d textures to load\n", numTextures);
    for (i32 i = 0; i < numTextures; ++i)
    {
        db->LoadTexture(db, textures[i], bVerbose);
    }
    
    // Textures need to be loaded before this point!
    ZRMaterial* mat = db->CreateMaterial(
        db,
        ZRDB_DEFAULT_DIFFUSE_MAT_NAME,
        //"data/W33_5.bmp",
        "data/debug_blue.png",
        "data/debug_black.png");
    
    // Create materials
    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_WORLD,
        "data/W33_5.bmp",
        "data/debug_black.png"
    );

    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_ENT,
        "data/debug_red.png",
        "data/debug_black.png"
    );

    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_PRJ,
        "data/debug_red_dark.png",
        "data/debug_black.png"
    );

    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_GFX,
        "data/debug_yellow.png",
        "data/debug_black.png"
    );
    
    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_LASER,
        "data/debug_red.png",
        "data/debug_black.png"
    );
}

///////////////////////////////////////////////////////////////////////////
// Create
///////////////////////////////////////////////////////////////////////////

extern "C" ZRAssetDB* ZRDB_Create()
{
    ZRAssetDBData* db = (ZRAssetDBData*)malloc(sizeof(ZRAssetDBData));
    *db = {};
    /////////////////////////////////////////////////////
    // functions
    db->nextId = ZR_ASSET_DB_FIRST_ID;

    // Get asset
    db->header.GetMeshByName = ZRDB_GetMeshByName;
    db->header.GetMeshHandleByName = ZRDB_GetMeshHandleByName;
    db->header.GetMeshByIndex = ZRDB_GetMeshByIndex;
    db->header.GetNumMeshes = ZRDB_GetNumMeshes;

    db->header.GetTextureByName = ZRDB_GetTextureByName;
    db->header.GetTextureHandleByIndex = ZRDB_GetTextureHandleByIndex;
    db->header.GetNumTextures = ZRDB_GetNumTextures;
    db->header.GetTextureByIndex = ZRDB_GetTextureByIndex;

    db->header.GetMaterialByName = ZRDB_GetMaterialByName;
    db->header.GetMaterialByIndex = ZRDB_GetMaterialByIndex;
    // Create
    db->header.CreateMaterial = ZRDB_CreateMaterial;
    // Load
    db->header.LoadMesh = ZRDB_LoadMesh;
    db->header.LoadMeshFromFBX = ZRDB_LoadMeshFromFBX;
    db->header.LoadTexture = ZRDB_LoadTexture;

    // store
    db->textures = (ZRDBTexture*)malloc(sizeof(ZRDBTexture) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxTextures = ZR_ASSET_DB_MAX_HANDLES;
    db->meshes = (ZRDBMesh*)malloc(sizeof(ZRDBMesh) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMeshes = ZR_ASSET_DB_MAX_HANDLES;
    db->materials = (ZRMaterial*)malloc(sizeof(ZRMaterial) * ZR_ASSET_DB_MAX_HANDLES);
    db->maxMaterials = ZR_ASSET_DB_MAX_HANDLES;

	ZRDB_LoadEmbedded(&db->header);

    return &db->header;
}

extern "C" void ZRDB_AttachUploader(ZRAssetDB* assetDB, ZRAssetUploader uploader)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    db->uploader = uploader;
}
#endif // ZR_ASSET_DB_CPP
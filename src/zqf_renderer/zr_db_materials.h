#ifndef ZR_DB_MATERIALS_H
#define ZR_DB_MATERIALS_H

#include "zr_asset_db.h"

static ZRMaterial* ZRDB_GetMaterialByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = 0;
    for (i32 i = 0; i < db->numMaterials; ++i)
    {
        if (ZE_CompareStrings(name, db->materials[i].name) == 0)
        {
            return &db->materials[i];
        }
    }
	// TODO: Assuming mat 0 is default:
	// No returning NULL!
	return &db->materials[0];
}

static ZRMaterial* ZRDB_GetMaterialByIndex(ZRAssetDB* assetDB, i32 index)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    if (index < 0 || index >= db->numMaterials) { index = 0; }
    return &db->materials[index];
}

static ZRMaterial* ZRDB_GetFreeMaterial(ZRAssetDB* assetDB, char* newName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	i32 index = db->numMaterials++;
	ZRMaterial* mat = &db->materials[index];
    *mat = {};
    mat->index = index;
	mat->name = newName;
	return mat;
}

static void ZRDB_CreateMaterial(
	ZRAssetDB* assetDB, char* name, char* diffuseName, char* emissiveName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	ZRMaterial* mat = ZRDB_GetFreeMaterial(assetDB, name);
	mat->diffuseTexIndex = ZRDB_GetTextureIndexByName(assetDB, diffuseName);
	mat->emissionTexIndex = ZRDB_GetTextureIndexByName(assetDB, emissiveName);
    printf("ZRDB Create material id %d: \"%s\"\n", mat->index, mat->name);
}

static void ZRDB_GetMaterialByIndex(
    ZRAssetDB* assetDB, i32 index, ZRMaterial* result)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	if (index < 0 || index >= db->numMaterials) { index = 0; }
    *result = db->materials[index];
}


#endif // ZR_DB_MATERIALS_H
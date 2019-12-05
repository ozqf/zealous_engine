#ifndef ZR_DB_MATERIALS_H
#define ZR_DB_MATERIALS_H

#include "zr_asset_db.h"

static i32 ZRDB_GetMaterialIndexByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = 0;
    for (i32 i = 0; i < db->numMaterials; ++i)
    {
        if (ZE_CompareStrings(name, db->materials[i].name) == 0)
        {
            index = i;
            break;
        }
    }
    return index;
}

static ZRMaterial* ZRDB_GetFreeMaterial(ZRAssetDB* assetDB, char* newName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	i32 index = db->numMaterials++;
	ZRMaterial* mat = &db->materials[index];
    mat->id = index;
	mat->name = newName;
	return mat;
}

static ErrorCode ZRDB_CreateMaterial(
	ZRAssetDB* assetDB, char* name, char* diffuseName, char* emissiveName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	ZRMaterial* mat = ZRDB_GetFreeMaterial(assetDB, name);
	mat->diffuseTexIndex = ZRDB_GetTexIndexByName(assetDB, diffuseName);
	mat->emissionTexIndex = ZRDB_GetTexIndexByName(assetDB, emissiveName);
    printf("ZRDB Create material id %d: \"%s\"\n", mat->id, mat->name);
    return ZE_ERROR_NONE;
}

static void ZRDB_GetMaterialByIndex(ZRAssetDB* assetDB, i32 index, ZRMaterial* result)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	if (index < 0 || index >= db->numMaterials) { index = 0; }
    *result = db->materials[index];
}


#endif // ZR_DB_MATERIALS_H
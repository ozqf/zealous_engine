#ifndef ZR_DB_MATERIALS_H
#define ZR_DB_MATERIALS_H

#include "../../headers/zr_asset_db.h"

static i32 ZRDB_GetNumMaterials(ZRAssetDB* handle)
{
    ZRDB_CAST_TO_INTERNAL(handle, db)
    return db->numMaterials;
}

static ZRDBMaterial* ZRDB_GetMaterialByName(ZRAssetDB* assetDB, char* name)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    i32 index = 0;
    for (i32 i = 0; i < db->numMaterials; ++i)
    {
        if (ZStr_Compare(name, db->materials[i].header.fileName) == 0)
        {
            return &db->materials[i];
        }
    }
	return &db->materials[ZRDB_DEFAULT_MAT_INDEX];
}

static ZRDBMaterial* ZRDB_GetMaterialByIndex(ZRAssetDB* assetDB, i32 index)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
    if (index < 0 || index >= db->numMaterials) { index = 0; }
    return &db->materials[index];
}

static ZRDBMaterial* ZRDB_GetFreeMaterial(ZRAssetDB* assetDB, char* newName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	i32 index = db->numMaterials++;
	ZRDBMaterial* mat = &db->materials[index];
    *mat = {};
    mat->header.index = index;
    mat->header.id = db->nextId++;
	mat->header.fileName = newName;
	return mat;
}

static ZRDBMaterial* ZRDB_CreateMaterial(
	ZRAssetDB* assetDB, char* name, char* diffuseName, char* emissiveName)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	ZRDBMaterial* mat = ZRDB_GetFreeMaterial(assetDB, name);
	mat->data.diffuseTexIndex = ZRDB_GetTextureIndexByName(assetDB, diffuseName);
	mat->data.emissionTexIndex = ZRDB_GetTextureIndexByName(assetDB, emissiveName);
    printf("ZRDB Create material id %d: \"%s\"\n",
        mat->header.index, mat->header.fileName);
    printf("\tDiffuse index %d, emission index %d\n",
        mat->data.diffuseTexIndex, mat->data.emissionTexIndex);
    return mat;
}

static void ZRDB_GetMaterialByIndex(
    ZRAssetDB* assetDB, i32 index, ZRDBMaterial* result)
{
    ZRDB_CAST_TO_INTERNAL(assetDB, db)
	if (index < 0 || index >= db->numMaterials) { index = 0; }
    *result = db->materials[index];
}

#endif // ZR_DB_MATERIALS_H
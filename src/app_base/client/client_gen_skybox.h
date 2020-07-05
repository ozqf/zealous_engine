#include "client_internal.h"
#include "../../zr_embedded/zr_embedded.h"

internal void CLDebug_NewMaterial(
    ZRAssetDB* db,
    i32 width,
    i32 height,
    char* matName,
    char* baseTexName,
    char* emitTextName,
    ZRMaterial** matResult,
    ZRDBTexture** baseResult,
    ZRDBTexture** emitResult)
{
    ZRDBTexture* base,* emit;
    ZRMaterial* mat;
    base = db->GenBlankTexture(db, baseTexName, width, height, { 255, 255, 255, 255 });
	emit = db->GenBlankTexture(db, emitTextName, width, height, { 0, 0, 0, 0 });
	mat = db->CreateMaterial(
        db,
        matName,
        baseTexName,
        emitTextName
    );
    *baseResult = base;
    *emitResult = emit;
    *matResult = mat;
}

internal ZRDrawObj* CLDebug_CreateBuilding(ZRAssetDB* db)
{
    ZRDrawObj* obj = &g_debugObjs[g_numDebugObjs];
    *obj = {};
    i32 cubeMeshIndex = db->GetMeshByName(db, ZRDB_MESH_NAME_CUBE)->header.index;
    i32 cityMat = db->GetMaterialByName(db, "city")->index;
    Transform_SetToIdentity(&obj->t);
    obj->data.SetAsMesh(cubeMeshIndex, cityMat);
    obj->t.pos.x = 64;
    obj->t.pos.z = 64;
    obj->t.pos.y = -15;
    obj->t.scale = { 12, 64, 12 };
    g_numDebugObjs++;
	return obj;
}

internal void CGen_CreateSkybox(ZRAssetDB* db)
{
	////////////////////////////////////////////////
    // grid
	ZRDBTexture* base,* emit;
    ZRMaterial* mat;

    //////////////////////////////////////////////////////
    // background cuboids
    CLDebug_NewMaterial(db, 32, 32, "city", "citybase", "cityemit", &mat, &base, &emit);
    TexGen_SetRGBA((ColourU32*)base->data, 32, 32, { 50, 200, 0, 255 });
    TexGen_SetRGBA((ColourU32*)emit->data, 32, 32, { 50, 200, 0, 255 });
    //TexGen_FillRect()
	CLDebug_CreateBuilding(db);
}

#include "client_internal.h"
#include "../../zr_embedded/zr_embedded.h"

internal void ZP_CreateRingScatter2D(
    Transform* xforms, i32 numXforms, Vec3 origin, f32 minRadius, f32 maxRadius)
{
    if (maxRadius <= 0) { maxRadius = 1; }
    if (minRadius < maxRadius) { minRadius = maxRadius; }
    for (i32 i = 0; i < numXforms; ++i)
    {
        f32 dist = COM_STDRandomInRange(minRadius, maxRadius);
        f32 radians = COM_STDRandomInRange(0, 360) * DEG2RAD;
        f32 x = cosf(radians) * dist;
        f32 z = sinf(radians) * dist;
        Transform_SetToIdentity(&xforms[i]);
        xforms[i].pos.x = x;
        xforms[i].pos.z = z;
    }
}

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
    TexGen_SetRGBA((ColourU32*)base->data, 32, 32, { 50, 200, 100, 255 });
    TexGen_SetRGBA((ColourU32*)emit->data, 32, 32, { 50, 200, 100, 0 });
    //TexGen_DrawFilledCircle((ColourU32*)emit->data, 32, 32, { 16, 16 }, 10, { 255, 255, 255, 255});
    TexGen_PaintHorizontalLines((ColourU32*)emit->data, 32, 32, 5, { 255, 0, 0, 255 });
    //TexGen_FillRect()
	CLDebug_CreateBuilding(db);

    Transform xforms[64];
    ZP_CreateRingScatter2D(xforms, 16, { 0, 0, 0 }, 30, 64);
    ZP_CreateRingScatter2D(&xforms[16], 16, { 0, 0, 0 }, 72, 128);
    for (i32 i = 0; i < 32; ++i)
    {
        Transform* t = &xforms[i];
        ZRDrawObj* obj = CLDebug_CreateBuilding(db);
        obj->t.pos.x = t->pos.x;
        obj->t.pos.z = t->pos.z;
    }
}

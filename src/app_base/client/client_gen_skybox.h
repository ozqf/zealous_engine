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

internal void PrintQuad(ZEQuad q)
{
	for (i32 i = 0; i < MESH_GEN_VERTS_PER_QUAD; ++i)
	{
		Vec3 v = q.verts[i];
		Vec2 uv = q.uvs[i];
		Vec3 normal = q.normals[i];
		printf("%.3f, %.3f, %.3f\n", v.x, v.y, v.z);
		printf("\t%.3f, %.3f\n", uv.x, uv.y);
		printf("\t%.3f, %.3f, %.3f\n", normal.x, normal.y, normal.z);
	}
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
	
	////////////////////////////////////////////////
    // Quad test
	printf("------------- QUAD TEST --------------\n");
	i32 numQuads = 4;
	i32 numVerts = MeshGen_NumVertsForQuads(numQuads);
	printf("Creating %d quads (%d verts)\n", numQuads, numVerts);
    printf("\tNum verts per quad %d\n", MESH_GEN_VERTS_PER_QUAD);
	ZRDBMesh* quadMesh = db->CreateEmptyMesh(db, "quad_gen", numVerts);
    ZEQuad q1, q2, q3, q4;
    q1 = MeshGen_SelectQuad(quadMesh->data, 0);
    MeshGen_ResetQuad(q1, { 2, 2 });
    MeshGen_SetSquareUVs(q1, 32, 32);

    q2 = MeshGen_SelectQuad(quadMesh->data, 1);
    MeshGen_ResetQuad(q2, { 2, 2 });
    q3 = MeshGen_SelectQuad(quadMesh->data, 2);
    MeshGen_ResetQuad(q3, { 2, 2 });
    q4 = MeshGen_SelectQuad(quadMesh->data, 3);
    MeshGen_ResetQuad(q4, { 2, 2 });

    ///////////////////////////////////////////////
    // Translate quads
    M4x4_CREATE(translate)
    M4x4_CREATE(rotate)
    f32 radius = 2;
    // position q1
    M4x4_BuildTranslation(translate.cells, 0, 0, radius);
    Vec3_MultiplyArrayByM4x4(q1.verts, MESH_GEN_VERTS_PER_QUAD, translate.cells);
    // position q2
    M4x4_BuildRotateByAxis(rotate.cells, 180 * DEG2RAD, 0, 1, 0);
    rotate.posZ = -radius;
    Vec3_MultiplyArrayByM4x4(q2.verts, MESH_GEN_VERTS_PER_QUAD, rotate.cells);
    // position q3
    M4x4_BuildRotateByAxis(rotate.cells, 90 * DEG2RAD, 0, 1, 0);
    rotate.posX = radius;
    Vec3_MultiplyArrayByM4x4(q3.verts, MESH_GEN_VERTS_PER_QUAD, rotate.cells);
    // position q4
    M4x4_BuildRotateByAxis(rotate.cells, 270 * DEG2RAD, 0, 1, 0);
    rotate.posX = -radius;
    Vec3_MultiplyArrayByM4x4(q4.verts, MESH_GEN_VERTS_PER_QUAD, rotate.cells);

    ///////////////////////////////////////////////
    // update mesh vert count and mark assets as dirty
    quadMesh->data.numVerts = MESH_GEN_VERTS_PER_QUAD * numQuads;
    quadMesh->header.bIsDirty = YES;
    db->bDirty = YES;
    quadMesh->data.PrintVerts();
    
    ///////////////////////////////////////////////
    // Create objects to display mesh
    Vec3 pos = { 0, 2, 0 };
    // mark quad obj position with a mesh that will render properly!
    ZRDrawObj* marker = &g_debugObjs[g_numDebugObjs++];
    ZRDrawObj_Clear(marker);
    marker->data.SetAsMesh(0, 0);
    marker->t.pos = pos;
    marker->t.scale = { 0.2f, 0.2f, 0.2f };

    ZRDrawObj* quadObj = &g_debugObjs[g_numDebugObjs++];
    ZRDrawObj_Clear(quadObj);
    quadObj->data.SetAsMesh(quadMesh->header.index, mat->index);
    //quadObj->data.SetAsMesh(0, mat->index);
    quadObj->t.pos = pos;
}

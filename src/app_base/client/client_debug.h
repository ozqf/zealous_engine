#ifndef CLIENT_DEBUG_H
#define CLIENT_DEBUG_H

#include "../../voxel_world/voxel_world.h"

static SimEntity g_debugEnts[16];

static ZRDrawObj g_debugObjs[16];
static i32 g_numDebugObjs = 0;

static void CLDebug_MakeVoxelWorld()
{
    VWChunk* chunk;
    VWError err = VW_AllocChunk(8, &chunk);
    if (err != 0)
    {
        printf("Error %d creating VWChunk\n", err);
        return;
    }
    printf("Made VWChunk size %d with %d blocks\n", chunk->size, chunk->numBlocks);
}

static void CLDebug_SetAsLine(ZRDrawObj* obj, Vec3 a, Vec3 b)
{

}

static void CLDebug_UpdateDebugObjects()
{
    ZRAssetDB* db = App_GetAssetDB();
    i32 meshIndex = db->GetMeshByName(db, ZRDB_MESH_NAME_CUBE)->header.index;
    i32 matIndex = db->GetMaterialByName(db, ZRDB_MAT_NAME_GFX)->index;
    i32 matIndex2 = db->GetMaterialByName(db, ZRDB_MAT_NAME_PRJ)->index;
    i32 matIndex3 = db->GetMaterialByName(db, ZRDB_MAT_NAME_ENT)->index;
    g_numDebugObjs = 0;
    
    // solid test for ent to touch
    ZRDrawObj* wall = &g_debugObjs[g_numDebugObjs];
    *wall = {};
    Transform_SetToIdentity(&wall->t);
    wall->data.SetAsMesh(meshIndex, matIndex);
    wall->t.pos.x = -5;
    wall->t.pos.z = -5;
    wall->t.pos.y = 1;
    g_numDebugObjs++;

    // touching ent
    ZRDrawObj* actor = &g_debugObjs[g_numDebugObjs];
    *actor = {};
    Transform_SetToIdentity(&actor->t);
    actor->data.SetAsMesh(meshIndex, matIndex2);
    actor->t.pos.x = 5;
    actor->t.pos.y = 3;
    g_numDebugObjs++;

    // line between
    Vec3 b = wall->t.pos;
    Vec3 a = actor->t.pos;
    Vec3 d = {};
    d.x = b.x - a.x;
    d.y = b.y - a.y;
    d.z = b.z - a.z;

    Vec3 angles = Vec3_EulerAnglesBetween(a, b);
    Vec3 pos = {};
    pos.x = a.x + (d.x / 2);
    pos.y = a.y + (d.y / 2);
    pos.z = a.z + (d.z / 2);

    ZRDrawObj* line = &g_debugObjs[g_numDebugObjs];
    *line = {};
    Transform_SetToIdentity(&line->t);
    line->data.SetAsMesh(meshIndex, matIndex3);
    M3x3_RotateY(line->t.rotation.cells, angles.y);
	M3x3_RotateX(line->t.rotation.cells, -angles.x);
    line->t.pos = pos;
    line->t.scale = { 0.2f, 0.2f, d.z * 2 };
    g_numDebugObjs++;

    //o->t.scale = { 2, 2, 2 };
    /* Testing actor movement:
    SimEnt_MoveVsSolid
        Sim_FindByRaycast // find all hits
        if overlaps > 0
        Sim_FindClosestRayhit
    */
}

#endif // CLIENT_DEBUG_H
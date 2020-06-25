#ifndef CLIENT_DEBUG_H
#define CLIENT_DEBUG_H

#include "../../voxel_world/voxel_world.h"

#define CL_DEBUG_CAMERA_MODE_FREE 0
#define CL_DEBUG_CAMERA_MODE_TOP_DOWN 1

static SimEntity g_debugEnts[16];

// debugging camera to fly around with
static SimActorInput g_debugInput = {};
static Transform g_debugCamera;
// records static camera position for top down debug
static Transform g_debugTopdownCamera;
static i32 g_debugCameraMode = CL_DEBUG_CAMERA_MODE_TOP_DOWN;

static ZRDrawObj g_debugObjs[16];
static i32 g_numDebugObjs = 0;

static void CLDebug_Init()
{
    VWChunk* chunk;
    VWError err = VW_AllocChunk(8, &chunk);
    if (err != 0)
    {
        printf("Error %d creating VWChunk\n", err);
        return;
    }
    printf("Made VWChunk size %d with %d blocks\n", chunk->size, chunk->numBlocks);

	Transform_SetToIdentity(&g_debugTopdownCamera);
    g_debugTopdownCamera.pos.z = 10;
    g_debugTopdownCamera.pos.y += 34;
    Transform_SetRotation(&g_debugTopdownCamera, -(80.0f    * DEG2RAD), 0, 0);

	g_debugCamera = g_debugTopdownCamera;

}

static void CLDebug_FlyCamera(Transform* t, f32 moveSpeed, f32 delta)
{

}

static void CLDebug_SetAsLine(ZRDrawObj* obj, Vec3 a, Vec3 b)
{

}

internal void CL_ProcessDebugInput(InputActionSet* actions, i64 platformFrame)
{
    i32 bPrintLightCounts = NO;
    #if 1
    if (Input_CheckActionToggledOn(actions, "Debug Forward", platformFrame))
    {
        //g_rendCfg.extraLightsMax++;
        g_rendCfg.worldLightsMax++;
        bPrintLightCounts = YES;
    }
    #endif
    #if 1
    if (Input_CheckActionToggledOn(actions, "Debug Backward", platformFrame))
    {
        g_rendCfg.worldLightsMax--;
        if (g_rendCfg.worldLightsMax < 0)
        {
            g_rendCfg.worldLightsMax = 0;
        }
        // g_rendCfg.extraLightsMax--;
        // if (g_rendCfg.extraLightsMax < 0)
        // {
        //     g_rendCfg.extraLightsMax = 0;
        // }
        bPrintLightCounts = YES;
    }
    #endif
	if (Input_CheckActionToggledOn(actions, "Debug Camera", platformFrame))
	{
		if (g_clDebugFlags & CL_DEBUG_FLAG_DEBUG_CAMERA)
		{
			g_clDebugFlags &= ~CL_DEBUG_FLAG_DEBUG_CAMERA;
		}
		else
		{
			g_clDebugFlags |= CL_DEBUG_FLAG_DEBUG_CAMERA;
		}
		
	}
    if (bPrintLightCounts == YES)
    {
        printf("CL max lights: world %d extra %d\n",
            g_rendCfg.worldLightsMax, g_rendCfg.extraLightsMax);
    }
}

static i32 CLDebug_IsDebugInputActive()
{
	if ((g_clDebugFlags & CL_DEBUG_FLAG_DEBUG_CAMERA)
		&& g_debugCameraMode == CL_DEBUG_CAMERA_MODE_FREE)
	{
		return YES;
	}
	return NO;
}

static void CLDebug_UpdateDebugObjects()
{
	// update debug input for fly camera
	CL_UpdateActorInput(&g_inputActions, &g_debugInput);

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
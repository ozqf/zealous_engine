#ifndef CLIENT_DEBUG_H
#define CLIENT_DEBUG_H

#include "client_internal.h"
#include "../../voxel_world/voxel_world.h"
#include "../../zr_embedded/zr_embedded.h"

static i32 CLDebug_IsDebugInputActive()
{
	if ((g_clDebugFlags & CL_DEBUG_FLAG_DEBUG_CAMERA)
		&& g_debugCameraMode == CL_DEBUG_CAMERA_MODE_FREE)
	{
		return YES;
	}
	return NO;
}

internal void CLDebug_GenAssets()
{
	printf("APP - generate assets\n");
	ZRAssetDB* db = (ZRAssetDB*)App_GetAssetDB();

    ////////////////////////////////////////////////
    // grid
	ZRDBTexture* base,* emit;
    ZRMaterial* mat;
	base = db->GenBlankTexture(db, "grid", 32, 32, { 155, 155, 155, 255 });
	TexGen_FillRect((ColourU32*)base->data, 32, 32, { 0, 0 }, { 16, 16 },
		{ 225, 225, 225, 255 });
	TexGen_FillRect((ColourU32*)base->data, 32, 32, { 16, 16 }, { 16, 16 },
		{ 225, 225, 225, 255 });
	
	emit = db->GenBlankTexture(db, "grid_emit", 32, 32, { 0, 0, 0, 0 });
	TexGen_FillRect((ColourU32*)emit->data, 32, 32, { 0, 0 }, { 4, 32 },
		{ 225, 225, 0, 255 });
	TexGen_FillRect((ColourU32*)emit->data, 32, 32, { 28, 0 }, { 4, 32 },
		{ 225, 225, 0, 255 });
	mat = db->CreateMaterial(
        db,
        "grid",
        "grid",
        "grid_emit"
    );
	
    ////////////////////////////////////////////////
    // dynamic cube
    MeshData* cube = ZR_Embed_Cube();
	ZRDBMesh* mesh = db->CreateEmptyMesh(db, "app_mesh", cube->numVerts);
    printf("Set directly as cube\n");
	mesh->data.CopyData(*cube);
    mesh->data.numVerts = 36;
    mesh->header.bIsDirty = YES;

    // Mark the asset db for re-uploading
    db->bDirty = YES;
}

static void CLDebug_InitDrawObjects()
{
    
    ZRAssetDB* db = App_GetAssetDB();
    i32 cubeMeshIndex = db->GetMeshByName(db, ZRDB_MESH_NAME_CUBE)->header.index;
    i32 meshIndex = db->GetMeshByName(db, "app_mesh")->header.index;
    i32 matIndex = db->GetMaterialByName(db, ZRDB_MAT_NAME_GFX)->index;
    i32 matIndex2 = db->GetMaterialByName(db, ZRDB_MAT_NAME_PRJ)->index;
    i32 matIndex3 = db->GetMaterialByName(db, ZRDB_MAT_NAME_ENT)->index;
    i32 cityMat = db->GetMaterialByName(db, "city")->index;
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
}

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
	g_debugInput.degrees.x = -80;

	g_debugCamera = g_debugTopdownCamera;

    CLDebug_GenAssets();
    CLDebug_InitDrawObjects();
}

static void CLDebug_FlyCamera(
	Transform* t, SimActorInput* input, f32 moveSpeed, timeFloat delta)
{
	
    // Apply Rotate
    M3x3_SetToIdentity(t->rotation.cells);
    M3x3_RotateY(t->rotation.cells, input->degrees.y * DEG2RAD);
    M3x3_RotateX(t->rotation.cells, input->degrees.x * DEG2RAD);

	Vec3 dirInput = {};
	if (input->buttons & ACTOR_INPUT_MOVE_FORWARD)
	{
		dirInput.z -= 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_BACKWARD)
	{
		dirInput.z += 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_LEFT)
	{
		dirInput.x -= 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_RIGHT)
	{
		dirInput.x += 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_UP)
	{
		dirInput.y += 1;
	}
	if (input->buttons & ACTOR_INPUT_MOVE_DOWN)
	{
		dirInput.y -= 1;
	}
	Vec3 forward = {};
	Vec3 left = {};
	Vec3 up = {};
	Vec3 move = {};
	forward.x = t->rotation.zAxis.x * dirInput.z;
	forward.y = t->rotation.zAxis.y * dirInput.z;
	forward.z = t->rotation.zAxis.z * dirInput.z;

	left.x = t->rotation.xAxis.x * dirInput.x;
	left.y = t->rotation.xAxis.y * dirInput.x;
	left.z = t->rotation.xAxis.z * dirInput.x;

	up.x = t->rotation.yAxis.x * dirInput.y;
	up.y = t->rotation.yAxis.y * dirInput.y;
	up.z = t->rotation.yAxis.z * dirInput.y;

	move.x = forward.x + left.x + up.x;
	move.y = forward.y + left.y + up.y;
	move.z = forward.z + left.z + up.z;

	Vec3_SetMagnitude(&move, moveSpeed);

	t->pos.x += (move.x * (f32)delta);
	t->pos.y += (move.y * (f32)delta);
	t->pos.z += (move.z * (f32)delta);
}

static void CLDebug_SetAsLine(ZRDrawObj* obj, Vec3 a, Vec3 b)
{
    
}

internal void CLD_UpdateDynamicMesh(timeFloat delta)
{
    static f32 time = 0;
    ZRDBMesh* base = ZRDB_GET_MESH_BY_NAME(App_GetAssetDB(), ZRDB_MESH_NAME_CUBE);
    ZRDBMesh* mesh = ZRDB_GET_MESH_BY_NAME(App_GetAssetDB(), "app_mesh");

    for (u32 i = 0; i < mesh->data.numVerts; ++i)
    {
        Vec3 src = *base->data.GetVert(i);
        Vec3* v = mesh->data.GetVert(i);
        v->x = src.x * (sinf(time));
        v->y = src.y * (sinf(time));
        v->z = src.z * (sinf(time));
    }
    mesh->header.bIsDirty = YES;
    App_GetAssetDB()->bDirty = YES;

    time += ((f32)delta * 2.f);
}

internal void CL_ProcessDebugInput(InputActionSet* actions, i64 platformFrame)
{
	if (CLDebug_IsDebugInputActive() == YES)
	{
		CL_UpdateActorInput(actions, &g_debugInput);
	}
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

static void CLDebug_UpdateDebugObjects(timeFloat delta)
{
    CLD_UpdateDynamicMesh(delta);
	// update debug input for fly camera
	CLDebug_FlyCamera(&g_debugCamera, &g_debugInput, 12, delta);

	// Text test
    #if 0
	ZRDrawObj* textObj = &g_debugObjs[g_numDebugObjs++];
	*textObj = {};
	Transform_SetToIdentity(&textObj->t);
	textObj->data.SetAsText(
		"Test\nText", -1, COLOUR_WHITE, ZR_TEXT_ALIGNMENT_TOP_LEFT);
    #endif

    //o->t.scale = { 2, 2, 2 };
    /* Testing actor movement:
    SimEnt_MoveVsSolid
        Sim_FindByRaycast // find all hits
        if overlaps > 0
        Sim_FindClosestRayhit
    */
}

internal void CL_WriteNetworkDebug(CharBuffer* str)
{
	//char* chars = str->chars;
	//i32 written = 0;
    str->cursor += sprintf_s(
        str->cursor,
        str->Space(),
        "CLIENT:\nSim Tick: %d\nElapsed: %.3f\nOutput Seq: %d\nAck Seq: %d\nDelay: %.3f\nJitter %.3f\n",
        CL_GetServerTick(), g_elapsed, g_acks.outputSequence,
		g_acks.remoteSequence, g_ping, g_jitter
    );


    str->cursor += sprintf_s(
            str->cursor,
            str->Space(),
			"=== Commands ===\n%d reliablebytes %d\n%d unreliable bytes %d\n",
            Stream_CountCommands(&g_reliableStream.inputBuffer).count,
            g_reliableStream.inputBuffer.Written(),
            Stream_CountCommands(&g_unreliableStream.inputBuffer).count,
            g_unreliableStream.inputBuffer.Written()
            );

    #if 0
    SimEntity* ent =  Sim_GetEntityBySerial(&g_sim, -1);
    if (ent)
    {
        written += sprintf_s(chars + written, str->maxLength,
			"World vol pos Y: %.3f\n", ent->t.pos.y);
    }
    #endif
	#if 0
	// currently overflows debug text buffer:
	for (i32 i = 0; i < ACK_CAPACITY; ++i)
	{
		AckRecord* rec = &g_acks.awaitingAck[i];
		if (rec->acked)
		{
			timeFloat time = rec->receivedTime - rec->sentTime;
			written += sprintf_s(chars + written, str->maxLength,
				"%.3f Sent: %.3f Rec: %.3f\n",
				time, rec->sentTime, rec->receivedTime
            );
		}
	}
	#endif
	//str->length = written;
}

internal void CL_WriteTransformDebug(CharBuffer* str)
{
	char* chars = str->chars;
	i32 written = 0;
    f32* m = g_matrix.cells;
    written += sprintf_s(chars, str->maxLength,
        "MATRIX:\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n%.3f, %.3f, %.3f, %.3f\n",
        m[0], m[4], m[8], m[12],
        m[1], m[5], m[9], m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15]
    );
}

internal void CL_WriteCameraDebug(CharBuffer* str)
{
	
}

extern "C" void CL_WriteDebugString(CharBuffer* str)
{
	CL_WriteNetworkDebug(str);
	//CL_WriteTransformDebug(str);
	//CL_WriteCameraDebug(str);
}

#endif // CLIENT_DEBUG_H
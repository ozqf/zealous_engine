#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#include "app_internal.h"
#include "../zr_embedded/zr_embedded.h"

internal void App_DebugInit() { }
#if 0
internal void App_DebugInit()
{
    RScene_Init(
        &g_debugScene, g_debugSceneItems, MAX_DEBUG_SCENE_ITEMS,
        90, RENDER_PROJECTION_MODE_IDENTITY, 8);
        
    g_debugStr.Set(g_debugStrBuffer, DEBUG_STRING_LENGTH);
}

internal void App_SetStringRenderObj(RendObj* obj, CharBuffer* str)
{
    
    //"textures\\charset_128x128.bmp"
    RendObj_SetAsAsciCharArray(
        obj,
        str->chars,
        str->Written(),
        0.04f,
        TEXT_ALIGNMENT_TOP_LEFT,
        Tex_GetTextureIndexByName("textures\\charset.bmp"),
        0, 1, 1
    );
}

internal void App_WriteSpeeds(CharBuffer* str)
{
    // app overall
    str->cursor += sprintf_s(
        str->cursor,
        str->Space(),
        "-APP TIME %.3fms-\nREND %.3fms\n",
        App_GetPerformanceTime(APP_STAT_FRAME_TOTAL),
        App_GetPerformanceTime(APP_STAT_RENDER_TOTAL)
    );
    // sv
    str->cursor += sprintf_s(
        str->cursor,
        str->Space(),
        "SV Speeds:\n\tInput %.3fms\n\tSim %.3fms\n\tOutput %.3fms\n",
        App_GetPerformanceTime(APP_STAT_SV_INPUT),
        App_GetPerformanceTime(APP_STAT_SV_SIM),
        App_GetPerformanceTime(APP_STAT_SV_OUTPUT)
    );
    // cl
    str->cursor += sprintf_s(
        str->cursor,
        str->Space(),
        "CL Speeds:\n\tInput %.3fms\n\tSim %.3fms\n\tOutput %.3fms\n\tRender %.3fms\n",
        App_GetPerformanceTime(APP_STAT_CL_INPUT),
        App_GetPerformanceTime(APP_STAT_CL_SIM),
        App_GetPerformanceTime(APP_STAT_CL_OUTPUT),
        App_GetPerformanceTime(APP_STAT_CL_RENDER)
    );
}

internal void App_WriteDebugStrings()
{
	g_debugScene.numObjects = 0;
    g_debugStr.Reset();
    // Transform position on screen
    Transform t = {};
    i32 posIndex = 0;
    CharBuffer speedsSub;
    CharBuffer serverSub;
    RenderListItem* r = NULL;
    if (g_debugPrintFlags & APP_PRINT_FLAG_SPEEDS)
    {
        speedsSub = g_debugStr.StartSubSection();
        App_WriteSpeeds(&speedsSub);
        g_debugStr.EndSubSection(&speedsSub);
        r = RScene_AssignNextItem(&g_debugScene);
        t.pos.x = g_debugStrPositions[posIndex].x;
        t.pos.y = g_debugStrPositions[posIndex].y;
        posIndex++;
        r->transform = t;
        App_SetStringRenderObj(&r->obj, &speedsSub);
    }

    if (g_debugPrintFlags & APP_PRINT_FLAG_SERVER)
    {
        serverSub = g_debugStr.StartSubSection();
        SV_WriteDebugString(&serverSub);
        g_debugStr.EndSubSection(&serverSub);
        r = RScene_AssignNextItem(&g_debugScene);
        t.pos.x = g_debugStrPositions[posIndex].x;
        t.pos.y = g_debugStrPositions[posIndex].y;
        posIndex++;
        r->transform = t;
        App_SetStringRenderObj(&r->obj, &serverSub);
    }

    if (g_debugPrintFlags & APP_PRINT_FLAG_CLIENT)
    {
        CharBuffer sub = g_debugStr.StartSubSection();
        CL_WriteDebugString(&sub);
        g_debugStr.EndSubSection(&sub);
        r = RScene_AssignNextItem(&g_debugScene);
        t.pos.x = g_debugStrPositions[posIndex].x;
        t.pos.y = g_debugStrPositions[posIndex].y;
        posIndex++;
        r->transform = t;
        App_SetStringRenderObj(&r->obj, &sub);
    }
    
}
#endif

internal void App_TickDebugUtils(timeFloat delta)
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

internal void App_GenAssets()
{
	printf("APP - generate assets\n");
	ZRAssetDB* db = (ZRAssetDB*)g_platform.GetAssetDB();

	ZRDBTexture* base,* emit;
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
	ZRMaterial* mat = db->CreateMaterial(
        db,
        "grid",
        "grid",
        "grid_emit"
    );
	
    MeshData* cube = ZR_Embed_Cube();
	ZRDBMesh* mesh = db->CreateEmptyMesh(db, "app_mesh", cube->numVerts);
    printf("Set directly as cube\n");
	mesh->data.CopyData(*cube);
    mesh->data.numVerts = 36;
    mesh->header.bIsDirty = YES;
    // Mark the asset db for re-uploading
    db->bDirty = YES;
}

#endif // APP_DEBUG_H
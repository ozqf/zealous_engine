#pragma once

#include "app_internal.h"

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

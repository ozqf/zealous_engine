#include "game_hud.h"



extern "C" void Hud_UpdateGameState(i32 gameRules, i32 gameState)
{
    
}

extern "C" void Hud_UpdatePlayerStatus(i32 health)
{
    
}

extern "C" void Hud_AddViewModels(
	ClientRenderer* cr,
    ZRViewFrame* frame,
    ZRSceneFrame* scene,
    ClientRenderSettings cfg)
{
	ZRDrawObj* obj = NULL;
    // Add View Model Scene
    if ((cfg.debugFlags & CL_DEBUG_FLAG_DEBUG_CAMERA) == 0)
    {
        //Vec3 placeholderScale = { 0.25f, 0.25f, 1 };
        Vec3 placeholderScale = { 1, 1, 1 };
        //i32 wallMesh = ZRDB_GET_MESH_BY_NAME(cr->db, ZRDB_MESH_NAME_CUBE)->header.index;
        //i32 wallMesh = ZRDB_GET_MESH_BY_NAME(cr->db, SIM_MODEL_GATLING_BARRELS)->header.index;
        //i32 wallMesh = cfg.viewModels.rightHandModelIndex;
        //i32 wallMat = ZRDB_GET_MAT_BY_NAME(cr->db, ZRDB_MAT_NAME_WORLD)->header.index;
        //i32 wallMat = cfg.viewModels.rightHandmatIndex;

        if (cfg.viewModels.rightHandModelIndex > 0)
        {
            #if 1 // right hand
            obj = ZRDrawObj_InitInPlace(&frame->list->cursor);
            obj->data.SetAsMesh(cfg.viewModels.rightHandModelIndex,
                cfg.viewModels.rightHandmatIndex);
            obj->t.scale = placeholderScale;
            scene->params.numObjects++;

            Vec3 posOffset = { 0.5f, -0.5f, -1.f };
            obj->t.pos.x = 0.5f;
            obj->t.pos.y = -0.5f;
            obj->t.pos.z = -1;
            Transform* links[2];
            links[0] = &scene->params.camera;
            links[1] = &obj->t;
            Transform_ApplyChain(&obj->t, links, 2);
            #endif
        }
        if (cfg.viewModels.leftHandModelIndex > 0)
        {
            #if 1 // left hand
            obj = ZRDrawObj_InitInPlace(&frame->list->cursor);
            obj->data.SetAsMesh(cfg.viewModels.leftHandModelIndex,
                cfg.viewModels.leftHandmatIndex);
            obj->t.pos.x = -0.5f;
            obj->t.pos.y = -0.5f;
            obj->t.pos.z = -1;
            obj->t.scale = placeholderScale;
            scene->params.numObjects++;
            #endif
        }
    }
}

internal void Hud_AddUI(ClientRenderer* cr, ZRViewFrame* frame, ClientRenderSettings cfg)
{
    ZRSceneFrame* scene = ZRScene_InitInPlace(frame->list, ZR_PROJECTION_MODE_ORTHO_BASE, NO);
	frame->numScenes++;
    Transform_SetToIdentity(&scene->params.camera);
    scene->params.camera.pos.z = 1;

    if (cfg.viewModels.textFieldFlags & CLR_HUD_ITEM_CROSSHAIR)
    {
        i32 wallMesh = ZRDB_GET_MESH_BY_NAME(cr->db, ZRDB_MESH_NAME_QUAD)->header.index;
        i32 wallMat = ZRDB_GET_MAT_BY_NAME(cr->db, ZRDB_MAT_NAME_CROSSHAIR)->header.index;
	    ZRDrawObj* obj = NULL;
	    obj = ZRDrawObj_InitInPlace(&frame->list->cursor);
        obj->data.SetAsMesh(wallMesh, wallMat);
        obj->t.pos.x = 0;
        obj->t.pos.y = 0;
        obj->t.pos.z = 0;
        obj->t.scale = { 0.1f, 0.1f, 0.1f };
        scene->params.numObjects++;
    }
	
    // Test status text
    // create a temp text draw obj
    if (cfg.viewModels.textFieldFlags & CLR_HUD_ITEM_PLAYER_STATUS
        && cfg.viewModels.health > 0)
    {
        ZE_BUILD_STRING(txt, 256, "HEALTH %d\nAMMO 999\n",
            cfg.viewModels.health)
        //char* txt = "HEALTH 100\nAMMO 999";
        ZRDrawObj txtObj = {};
        txtObj.data.SetAsText(
            txt, -1, COLOUR_WHITE, COLOUR_EMPTY, ZR_TEXT_ALIGNMENT_TOP_LEFT);
        txtObj.data.text.linesPerScreen = 32;
        // push object toward camera slightly, away from background
        txtObj.t.pos.x = -1.5f;
        txtObj.t.pos.y = -0.5f;
        // TODO: Depth currently doesn't work for text!
        txtObj.t.pos.z -= 0.5f;

        // add to draw list
        ZR_WriteTextObj(&txtObj, frame->list, frame->data);
        scene->params.numObjects++;
    }
    if (cfg.viewModels.textFieldFlags & CLR_HUD_ITEM_SPAWN_PROMPT)
    {
        char* txt = "PRESS SPACE TO SPAWN";
        ZRDrawObj txtObj = {};
        txtObj.data.SetAsText(
            txt, -1, COLOUR_WHITE, COLOUR_EMPTY, ZR_TEXT_ALIGNMENT_CENTRE);
        txtObj.data.text.linesPerScreen = 16;
        // push object toward camera slightly, away from background
        txtObj.t.pos.x = 0;
        txtObj.t.pos.y = -0.8f;
        // TODO: Depth currently doesn't work for text!
        txtObj.t.pos.z -= 0.5f;

        // add to draw list
        ZR_WriteTextObj(&txtObj, frame->list, frame->data);
        scene->params.numObjects++;
    }
    if (cfg.viewModels.textFieldFlags & CLR_HUD_ITEM_TITLE)
    {
        //i32 textureIndex = -1;
        i32 textureIndex = ZRDB_GET_TEX_BY_NAME(cr->db, SIM_TEX_CHARSET)->header.index;
        char* txt = "ZEALOUS ENGINE";
        ZRDrawObj txtObj = {};
        txtObj.data.SetAsText(
            txt, textureIndex, COLOUR_WHITE, COLOUR_EMPTY, ZR_TEXT_ALIGNMENT_CENTRE);
        txtObj.data.text.linesPerScreen = 12;
        // push object toward camera slightly, away from background
        txtObj.t.pos.x = 0;
        txtObj.t.pos.y = 0.5f;
        // TODO: Depth currently doesn't work for text!
        txtObj.t.pos.z -= 0.5f;

        // add to draw list
        ZR_WriteTextObj(&txtObj, frame->list, frame->data);
        scene->params.numObjects++;
    }
    // finish
    scene->params.numListBytes = frame->list->cursor - (u8*)scene->params.objects;
}

extern "C" void Hud_AddDrawObjects(
	ClientRenderer* cr, ZRViewFrame* frame, ClientRenderSettings cfg)
{
    #if 0
    ZRSceneFrame* scene = ZRScene_InitInPlace(frame->list, ZR_PROJECTION_MODE_3D, YES);
    Transform_SetToIdentity(&scene->params.camera);
	frame->numScenes++;
	Hud_AddViewModels(cr, frame, scene, cfg);
    scene->params.numListBytes = frame->list->cursor - (u8*)scene->params.objects;
    #endif

	Hud_AddUI(cr, frame, cfg);
}

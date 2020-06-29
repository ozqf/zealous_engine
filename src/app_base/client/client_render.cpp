#ifndef CLIENT_RENDER_CPP
#define CLIENT_RENDER_CPP

#include "client_render.h"
#include "../../zqf_renderer.h"
#include "../../ui/zui.h"

static ZRAssetDB* g_assetDb = NULL;

// static ZRDrawObj* CLR_InitDrawObjInPlace(u8** ptr)
// {
//     ZRDrawObj* obj = (ZRDrawObj*)*ptr;
//     *ptr += sizeof(ZRDrawObj);
//     *obj = {};
//     return obj;
// }

extern "C" void CLR_Init(ZRAssetDB* assetDb)
{
    g_assetDb = assetDb;
	ZUI_Init(assetDb);
}

extern "C" void CLR_Shutdown()
{
    g_assetDb = NULL;
}

/**
 * returns number of objects added
 */
internal i32 CLR_Debug_AddSimObjectsToRenderScene(
    SimScene* sim,
    ZEByteBuffer* list,
    ZEByteBuffer* scratch)
{
    // TODO: Look these up in asset db!
    ZRDBMesh* cube = g_assetDb->GetMeshByName(g_assetDb, ZRDB_MESH_NAME_CUBE);
    ZRDBMesh* quad = g_assetDb->GetMeshByName(g_assetDb, ZRDB_MESH_NAME_QUAD);
    //ZRDBTexture* defaultDiffuse

    i32 cubeIndex = cube->header.index;
    i32 quadIndex = quad->header.index;
    i32 defaultMaterialIndex = 0;
    i32 prjMaterialIndex = 2;

    i32 materialIndex = defaultMaterialIndex;

    i32 objCount = 0;
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }

        switch (ent->factoryType)
        {
            case SIM_FACTORY_TYPE_PROJ_PREDICTION:
            case SIM_FACTORY_TYPE_PROJECTILE_BASE:
            case SIM_FACTORY_TYPE_PROJ_PLAYER:
            materialIndex = prjMaterialIndex;
            case SIM_FACTORY_TYPE_BOUNCER:
            case SIM_FACTORY_TYPE_WANDERER:
            case SIM_FACTORY_TYPE_DART:
            case SIM_FACTORY_TYPE_SEEKER:
            case SIM_FACTORY_TYPE_ACTOR:
            case SIM_FACTORY_TYPE_EXPLOSION:
            case SIM_FACTORY_TYPE_BULLET_IMPACT:
            {
                ZRDrawObj* obj = ZRDrawObj_InitInPlace(&list->cursor);
                obj->data.SetAsMesh(cubeIndex, materialIndex);
                //ZRDrawObj_SetAsMesh(obj, cubeIndex, defaultMaterialIndex);
                //ZRDrawObj_SetAsPrefab(obj, ZR_PREFAB_TYPE_DEBUG_BOUNDING_BOX);
                obj->t = ent->body.t;
            } break;
        }
        objCount++;
    }
    return objCount;
}

/**
 * returns number of objects added
 */
internal i32 CLR_AddSimObjectsToRenderScene(
    SimScene* sim,
    Transform* camera,
    ZEByteBuffer* list,
    ZEByteBuffer* scratch,
    ClientRenderSettings cfg)
{
    ZRAssetDB* db = App_GetAssetDB();
    // ZRDBMesh* mesh;
    // ZRMaterial* mat;
    // mesh = db->GetMeshByName(db, "Cube");
    // i32 meshIndex = mesh->header.index;
    // mat = db->GetMaterialByName(db, ZRDB_DEFAULT_DIFFUSE_MAT_NAME);
    // i32 materialIndex = mat->index;

    if (cfg.worldLightsMax <= 0) { cfg.worldLightsMax = 4; }
    if (cfg.extraLightsMax <= 0) { cfg.extraLightsMax = 4; }

    i32 worldLights = cfg.worldLightsMax;
    i32 extraLights = cfg.extraLightsMax;
    
    i32 objCount = 0;
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
        if (ent->display.data.type == ZR_DRAWOBJ_TYPE_NONE) { continue; }
        i32 rendObjectsAdded = 0;
        
        switch (ent->display.data.type)
        {
            case ZR_DRAWOBJ_TYPE_MESH:
            {
                // straight make a copy of the entity's draw data.
                ZRDrawObj* obj = ZRDrawObj_InitInPlace(&list->cursor);
                obj->data = ent->display.data;
                
                obj->t = ent->body.t;
                rendObjectsAdded++;

                // add an optional light source
                if ((ent->factoryType == SIM_FACTORY_TYPE_PROJ_PLAYER
                    || ent->factoryType == SIM_FACTORY_TYPE_BULLET_IMPACT)
                    && extraLights > 0)
                {
                    obj = ZRDrawObj_InitInPlace(&list->cursor);
                    obj->data.SetAsPointLight({ 1, 1, 0 }, 1, 1);
                    obj->t = ent->body.t;
                    rendObjectsAdded++;
                    extraLights--;
                }
            } break;
            case ZR_DRAWOBJ_TYPE_DIRECT_LIGHT:
            case ZR_DRAWOBJ_TYPE_POINT_LIGHT:
            {
                if (worldLights > 0)
                {
                    ZRDrawObj* obj = ZRDrawObj_InitInPlace(&list->cursor);
                    obj->data = ent->display.data;
                    obj->t = ent->body.t;
                    rendObjectsAdded++;
                }
                worldLights--;
            } break;
        }
        objCount += rendObjectsAdded;
    }
    return objCount;
}

extern "C" void CLR_WriteDrawFrame(
    ZRViewFrame* frame,
    SimScene* sim,
    Transform* camera,
    ZRDrawObj* debugObjs,
    i32 numDebugObjs,
    ClientRenderSettings cfg)
{
    i32 requiredCapacity = sizeof(ZRViewFrame) + (sizeof(ZRDrawObj) * sim->maxEnts);

    ZEByteBuffer* list = frame->list;
    ZEByteBuffer* data = frame->data;

    if (cfg.debugFlags & CL_DEBUG_FLAG_VERBOSE_FRAME)
    {
        frame->bVerbose = YES;
        cfg.debugFlags &= ~CL_DEBUG_FLAG_VERBOSE_FRAME;
    }

    ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
    scene->params.bDeferred = YES;
    scene->params.bIsInteresting = NO;
    scene->params.bSkybox = YES;
    scene->params.projectionMode = ZR_PROJECTION_MODE_3D;
    scene->params.camera = *camera;
    i32 objCount = 0;
    //u8* listStart = list->cursor;
    scene->params.objects = (ZRDrawObj*)list->cursor;
    // write client draw data. World, view model, HUD, menus.

    #if 0 // DEBUG: Add a main light or objects are invisible
    ZRDrawObj* light = CLR_InitDrawObjInPlace(&list->cursor);
    objCount++;
    ZRDrawObj_SetAsPointLight(light, { 0, 1, 0 }, 2, 999.f);
    light->data.light.bCastShadows = YES;
    Transform_SetToIdentity(&light->t);
    light->t.pos.x = 0;// -20;
    light->t.pos.y = 5;// 20;
    light->t.pos.z = 0;// 20;
    Transform_SetRotation(&light->t, -45 * DEG2RAD, -45 * DEG2RAD, 0);
    #endif
    //////////////////////////////////////////////////
    // For debugging local listen servers ONLY!
    //////////////////////////////////////////////////
    if (cfg.debugFlags & CL_DEBUG_FLAG_DRAW_LOCAL_SERVER)
    {
        SimScene* serverSim;
        App_Debug_GetServerSim((void**)&serverSim);
        if (serverSim != NULL)
        {
            objCount += CLR_Debug_AddSimObjectsToRenderScene(serverSim, list, data);
        }
    }

    objCount += CLR_AddSimObjectsToRenderScene(sim, camera, list, data, cfg);

    for (i32 i = 0; i < numDebugObjs; ++i)
    {
        ZRDrawObj* obj = &debugObjs[i];
        list->cursor += ZE_COPY_STRUCT(obj, list->cursor, ZRDrawObj);
        objCount++;
		if (obj->data.type == ZR_DRAWOBJ_TYPE_TEXT)
		{
			ZE_ASSERT(data->Space() >= obj->data.text.length, "No space for string in draw data buffer")
			// copy string to data buffer
			data->cursor += ZE_Copy(
				(char*)data->cursor, (char*)obj->data.text.text, obj->data.text.length);
		}
        // printf("Added debug obj at %.3f, %.3f, %.3f\n",
        //     obj->t.pos.x, obj->t.pos.y, obj->t.pos.z);
    }

    scene->params.numDataBytes = list->cursor - (u8*)scene->params.objects;
    scene->params.numObjects = objCount;
    scene->sentinel = ZR_SENTINEL;
    frame->numScenes++;
}

#endif // CLIENT_RENDER_CPP
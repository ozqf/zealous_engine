#ifndef CLIENT_RENDER_CPP
#define CLIENT_RENDER_CPP

#include "client_render.h"
#include "../../zqf_renderer.h"

static ZRDrawObj* CLR_InitDrawObjInPlace(u8** ptr)
{
    ZRDrawObj* obj = (ZRDrawObj*)*ptr;
    *ptr += sizeof(ZRDrawObj);
    *obj = {};
    return obj;
}

extern "C" void CLR_Init()
{

}

extern "C" void CLR_Shutdown()
{
    
}

/**
 * returns number of objects added
 */
internal i32 CLR_Debug_AddSimObjectsToRenderScene(
    SimScene* sim,
    ZEByteBuffer* list,
    ZEByteBuffer* data)
{
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
            case SIM_FACTORY_TYPE_BOUNCER:
            case SIM_FACTORY_TYPE_WANDERER:
            case SIM_FACTORY_TYPE_DART:
            case SIM_FACTORY_TYPE_SEEKER:
            case SIM_FACTORY_TYPE_ACTOR:
            case SIM_FACTORY_TYPE_EXPLOSION:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_BOUNDING_BOX);
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
    ZEByteBuffer* list,
    ZEByteBuffer* data,
    u32 debugFlags)
{
    i32 objCount = 0;
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
        i32 rendObjectsAdded = 0;
        #if 0
        ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
            ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_PLAYER);
            obj->t = ent->body.t;
        objCount++;
        #endif
        #if 1
        switch (ent->factoryType)
        {
            case SIM_FACTORY_TYPE_PROJ_PREDICTION:
            case SIM_FACTORY_TYPE_PROJECTILE_BASE:
            case SIM_FACTORY_TYPE_PROJ_PLAYER:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_PLAYER_PROJECTILE);
                obj->t = ent->body.t;
                rendObjectsAdded++;
            } break;
            case SIM_FACTORY_TYPE_WORLD:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_WALL);
                obj->t = ent->body.t;
                rendObjectsAdded++;
            } break;
            case SIM_FACTORY_TYPE_BOUNCER:
            case SIM_FACTORY_TYPE_WANDERER:
            case SIM_FACTORY_TYPE_DART:
            case SIM_FACTORY_TYPE_SEEKER:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_ENEMY);
                obj->t = ent->body.t;
                rendObjectsAdded++;
            } break;
            case SIM_FACTORY_TYPE_BOT:
            case SIM_FACTORY_TYPE_ACTOR:
            {
                ZRDrawObj* obj;
                if (debugFlags & CL_DEBUG_FLAG_DRAW_REAL_LOCAL_POSITION)
                {
                    obj = CLR_InitDrawObjInPlace(&list->cursor);
                    ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_ITEM);
                    rendObjectsAdded++;
                    obj->t = ent->body.t;
                }
                obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_PLAYER);
                rendObjectsAdded++;
                if (debugFlags & CL_DEBUG_FLAG_NO_PLAYER_SMOOTHING)
                {
                    obj->t = ent->body.t;
                }
                else
                {
                    // calculate smoothed position
                    Vec3 pos = ent->body.t.pos;
                    pos.x -= ent->body.error.x;
                    pos.y -= ent->body.error.y;
                    pos.z -= ent->body.error.z;
                    ent->body.error.x *= ent->body.errorRate;
                    ent->body.error.y *= ent->body.errorRate;
                    ent->body.error.z *= ent->body.errorRate;

                    Vec3 errorPos = ent->body.error;
                    //printf("CLR Actor error %.3f, %.3f, %.3f\n",
                    //    errorPos.x, errorPos.y, errorPos.z);
                    obj->t.pos = pos;
                    obj->t.rotation = ent->body.t.rotation;
                    obj->t.scale = ent->body.t.scale;
                }
            } break;
            case SIM_FACTORY_TYPE_EXPLOSION:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_DEBUG_EXPLOSION);
                obj->t = ent->body.t;
                rendObjectsAdded++;
            } break;
        }
        objCount += rendObjectsAdded;
        #endif
    }
    return objCount;
}

extern "C" void CLR_WriteDrawFrame(
    ZEByteBuffer* list,
    ZEByteBuffer* data,
    SimScene* sim,
    Transform* camera,
    u32 debugFlags)
{
    i32 requiredCapacity = sizeof(ZRViewFrame) + (sizeof(ZRDrawObj) * sim->maxEnts);

    ZRViewFrame* frame = (ZRViewFrame*)list->cursor;
    list->cursor += sizeof(ZRViewFrame);
    *frame = {};
    ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
    scene->params.bDeferred = YES;
    scene->params.bIsInteresting = NO;
    scene->params.bSkybox = YES;
    scene->params.projectionMode = ZR_PROJECTION_MODE_3D;
    scene->params.camera = *camera;
    i32 objCount = 0;
    u8* listStart = list->cursor;
    // write client draw data. World, view model, HUD, menus.

    // DEBUG: Add a main light or objects are invisible
    ZRDrawObj* light = CLR_InitDrawObjInPlace(&list->cursor);
    objCount++;
    ZRDrawObj_SetAsPointLight(NULL, light, { 1, 1, 1 }, 999.f);
    light->data.light.bCastShadows = YES;
    Transform_SetToIdentity(&light->t);
    light->t.pos.x = -10;
    light->t.pos.y = 15;
    light->t.pos.z = 10;
    Transform_SetRotation(&light->t, -45 * DEG2RAD, -55 * DEG2RAD, 0);

    //////////////////////////////////////////////////
    // For debugging local listen servers ONLY!
    //////////////////////////////////////////////////
    if (debugFlags & CL_DEBUG_FLAG_DRAW_LOCAL_SERVER)
    {
        SimScene* serverSim;
        App_Debug_GetServerSim((void**)&serverSim);
        if (serverSim != NULL)
        {
            objCount += CLR_Debug_AddSimObjectsToRenderScene(serverSim, list, data);
        }
    }

    objCount += CLR_AddSimObjectsToRenderScene(sim, list, data, debugFlags);

    scene->params.dataBytes = list->cursor - listStart;
    scene->params.numObjects = objCount;
    scene->sentinel = ZR_SENTINEL;
    frame->sentinel = ZR_SENTINEL;
    frame->numScenes++;
}

#endif // CLIENT_RENDER_CPP
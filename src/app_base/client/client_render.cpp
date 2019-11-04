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

extern "C" void CLR_WriteDrawFrame(
    ZEByteBuffer* list,
    ZEByteBuffer* data,
    SimScene* sim,
    Transform* camera)
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
    scene->params.bSkybox = NO;
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

    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }

        switch (ent->factoryType)
        {
            case SIM_FACTORY_TYPE_PROJ_PLAYER:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_SPIKE);
                obj->t = ent->body.t;
            } break;

            case SIM_FACTORY_TYPE_WORLD:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_WALL);
                obj->t = ent->body.t;
            } break;
            case SIM_FACTORY_TYPE_BOUNCER:
            case SIM_FACTORY_TYPE_WANDERER:
            case SIM_FACTORY_TYPE_DART:
            case SIM_FACTORY_TYPE_SEEKER:
            case SIM_FACTORY_TYPE_ACTOR:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_WALL);
                obj->t = ent->body.t;
            } break;
            case SIM_FACTORY_TYPE_EXPLOSION:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_WALL);
                obj->t = ent->body.t;
            } break;
        }
        objCount++;
    }
    scene->params.dataBytes = list->cursor - listStart;
    scene->params.numObjects = objCount;
    scene->sentinel = ZR_SENTINEL;
    frame->sentinel = ZR_SENTINEL;
    frame->numScenes++;
}

#endif // CLIENT_RENDER_CPP
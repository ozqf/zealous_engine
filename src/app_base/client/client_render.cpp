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
    SimScene* sim)
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
    i32 objCount = 0;
    // write client draw data. World, view model, HUD, menus.
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }

        switch (ent->factoryType)
        {
            case SIM_FACTORY_TYPE_ACTOR:
            {
                ZRDrawObj* obj = CLR_InitDrawObjInPlace(&list->cursor);
                ZRDrawObj_SetAsModel(NULL, obj, ZR_PREFAB_TYPE_WALL);
                obj->t = ent->body.t;
            } break;
        }
        objCount++;
    }
    scene->params.numObjects = objCount;
}

#endif // CLIENT_RENDER_CPP
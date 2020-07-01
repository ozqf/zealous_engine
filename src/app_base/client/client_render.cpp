#ifndef CLIENT_RENDER_CPP
#define CLIENT_RENDER_CPP

#include "client_render.h"
#include "../../zqf_renderer.h"
#include "../../ui/zui.h"

static ZRAssetDB* g_assetDb = NULL;

#define CLR_NUM_TEST_PARTICLES 128
static ZRParticle g_testParticles[CLR_NUM_TEST_PARTICLES];
static ZRParticleEmitter g_testEmit;

extern "C" void CLR_Init(ZRAssetDB* assetDb)
{
    g_assetDb = assetDb;
	ZUI_Init(assetDb);
    g_testEmit = {};
    g_testEmit.particles = g_testParticles;
    g_testEmit.maxParticles = CLR_NUM_TEST_PARTICLES;
    g_testEmit.def.duration = 0.2f;
    g_testEmit.def.materialIndex = 0;
}

extern "C" void CLR_Shutdown()
{
    g_assetDb = NULL;
}

extern "C" void CLR_SpawnTestParticle(Vec3 pos, Vec3 vel)
{
    if (g_testEmit.numParticles >= g_testEmit.maxParticles)
    { return; }
    ZRParticle* p = &g_testEmit.particles[g_testEmit.numParticles++];
    p->tick = g_testEmit.def.duration;
    p->pos = pos;
    p->velocity = vel;
}

extern "C" void CLR_TickTestParticles(timeFloat delta)
{
    // TODO possible to update and cull in the same iteration?

    // iterate and update
    for (i32 i = 0; i < g_testEmit.numParticles; ++i)
    {
        ZRParticle* p = &g_testEmit.particles[i];
        ///////////////////////////////////////////////
        // Check for recycle
        if (p->tick <= 0)
        {
            // mark for recycle
            p->bCull = YES;
            continue;
        }
        p->tick -= (f32)delta;
        p->pos.x += p->velocity.x * (f32)delta;
        p->pos.y += p->velocity.y * (f32)delta;
        p->pos.z += p->velocity.z * (f32)delta;
    }
    // iterate again and cull
    for (i32 i = 0; i < g_testEmit.numParticles; ++i)
    {
        ZRParticle* p = &g_testEmit.particles[i];
        if (p->bCull == NO) { continue; }
        // disable flag
        p->bCull = NO;
        // cull
        i32 lastIndex = g_testEmit.numParticles - 1;
        if (i == 0 && g_testEmit.numParticles == 1)
        {
            // last active particle, just reset list
            g_testEmit.numParticles = 0;
        }
        else if (i == lastIndex)
        {
            // last in array, just truncate
            g_testEmit.numParticles--;
        }
        else
        {
            // swap and truncate
            ZRParticle* last = &g_testEmit.particles[lastIndex];
            *p = *last;
            g_testEmit.numParticles--;
        }
    }
}

internal void CLR_AddTestParticles(ZRSceneFrame* scene, ZEByteBuffer* list, ZEByteBuffer* data)
{
    ZRMaterial* mat = g_assetDb->GetMaterialByName(g_assetDb, ZRDB_MAT_NAME_GFX);
    for (i32 i = 0; i < g_testEmit.numParticles; ++i)
    {
        ZRParticle* p = &g_testEmit.particles[i];
        ZRDrawObj* obj = ZRDrawObj_InitInPlace(&list->cursor);
        scene->params.numObjects++;
        obj->data.SetAsMesh(0, mat->index);
        obj->t.pos = p->pos;
        obj->t.scale = { 0.2f, 0.2f, 0.2f };
    }
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
                    obj->data.SetAsPointLight({ 1, 1, 0 }, 1, 15);
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
    i32 objCount = 0;
    i32 requiredCapacity = sizeof(ZRViewFrame) + (sizeof(ZRDrawObj) * sim->maxEnts);
    ZRSceneFrame* scene = NULL;
    ZRDrawObj* obj = NULL;

    ZEByteBuffer* list = frame->list;
    ZEByteBuffer* data = frame->data;
    
    if (cfg.debugFlags & CL_DEBUG_FLAG_VERBOSE_FRAME)
    {
        frame->bVerbose = YES;
        cfg.debugFlags &= ~CL_DEBUG_FLAG_VERBOSE_FRAME;
    }

    scene = ZRSccene_InitInPlace(frame->list, ZR_PROJECTION_MODE_3D, YES);
    scene->params.camera = *camera;
    frame->numScenes++;
    objCount = 0;
    
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
        obj = &debugObjs[i];
        list->cursor += ZE_COPY_STRUCT(obj, list->cursor, ZRDrawObj);
        objCount++;
		if (obj->data.type == ZR_DRAWOBJ_TYPE_TEXT)
		{
			ZE_ASSERT(data->Space() >= obj->data.text.length, "No space for string in draw data buffer")
			// copy string to data buffer
			data->cursor += ZE_Copy(
				(char*)data->cursor, (char*)obj->data.text.text, obj->data.text.length);
		}
    }
    scene->params.numObjects = objCount;

    // Add test particles
    CLR_AddTestParticles(scene, list, data);
    scene->params.numDataBytes = list->cursor - (u8*)scene->params.objects;
    
    
    // Add View Model Scene
    if ((cfg.debugFlags & CL_DEBUG_FLAG_DEBUG_CAMERA) == 0)
    {
        #if 1 // right hand
        scene = ZRSccene_InitInPlace(frame->list, ZR_PROJECTION_MODE_3D, NO);
        Transform_SetToIdentity(&scene->params.camera);
        frame->numScenes++;
        obj = ZRDrawObj_InitInPlace(&list->cursor);
        obj->data.SetAsMesh(0, 0);
        obj->t.pos.x = 0.5f;
        obj->t.pos.y = -0.5f;
        obj->t.pos.z = -1;
        obj->t.scale = { 0.25f, 0.25f, 1 };
        scene->params.numObjects++;
        #endif
        #if 0 // left hand
        obj = ZRDrawObj_InitInPlace(&list->cursor);
        obj->data.SetAsMesh(0, 0);
        obj->t.pos.x = -0.5f;
        obj->t.pos.y = -0.5f;
        obj->t.pos.z = -1;
        obj->t.scale = { 0.25f, 0.25f, 1 };
        scene->params.numObjects++;
        #endif
        scene->params.numDataBytes = list->cursor - (u8*)scene->params.objects;
    }
}

#endif // CLIENT_RENDER_CPP
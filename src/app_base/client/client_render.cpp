#ifndef CLIENT_RENDER_CPP
#define CLIENT_RENDER_CPP

#include "client_render.h"
#include "../../zqf_renderer.h"
#include "../../ui/zui.h"

static ZRAssetDB* g_assetDb = NULL;

#define CLR_NUM_TEST_PARTICLES 128
static ZRParticle g_testParticles[CLR_NUM_TEST_PARTICLES];
static ZRParticleEmitter g_testEmit;

static ZRParticle g_gibParticles[CLR_NUM_TEST_PARTICLES];
static ZRParticleEmitter g_gibEmit;

extern "C" void CLR_Init(ZRAssetDB* assetDb)
{
    g_assetDb = assetDb;
	ZUI_Init(assetDb);

    i32 quadIndex = ZRDB_GET_MESH_BY_NAME(g_assetDb, ZRDB_MESH_NAME_QUAD)->header.index;

    g_testEmit = {};
    g_testEmit.particles = g_testParticles;
    g_testEmit.maxParticles = CLR_NUM_TEST_PARTICLES;
    g_testEmit.def.duration = 0.2f;
    g_gibEmit.def.startScale = { 0.5f, 0.5f, 0.5f };
    g_testEmit.def.materialIndex = ZRDB_GET_MAT_BY_NAME(g_assetDb, ZRDB_MAT_NAME_PRJ)->index;
    g_testEmit.def.meshIndex = quadIndex;

    g_gibEmit = {};
    g_gibEmit.particles = g_gibParticles;
    g_gibEmit.maxParticles = CLR_NUM_TEST_PARTICLES;
    g_gibEmit.def.billboard = YES;
    g_gibEmit.def.duration = 1;
    g_gibEmit.def.startScale = { 1, 1, 1 };
    g_gibEmit.def.materialIndex = ZRDB_GET_MAT_BY_NAME(g_assetDb, ZRDB_MAT_NAME_PRJ)->index;
    g_gibEmit.def.meshIndex = quadIndex;
    g_gibEmit.def.pull = { 0, -40, 0 };
}

extern "C" void CLR_Shutdown()
{
    g_assetDb = NULL;
}

internal ZRParticle* CLR_GetFreeParticle(ZRParticleEmitter* emitter)
{
    if (emitter->numParticles >= emitter->maxParticles) { return NULL; }
    ZRParticle* p = &emitter->particles[emitter->numParticles++];
    return p;
}

extern "C" void CLR_SpawnTestParticle(i32 type, Vec3 pos, Vec3 vel)
{
    if (g_testEmit.numParticles >= g_testEmit.maxParticles)
    { return; }
    ZRParticleEmitter* emitter = NULL;
    switch (type)
    {
        case CLR_PARTICLE_TYPE_GIB: emitter = &g_gibEmit; break;
        default: emitter = &g_testEmit; break;
    }
    ZRParticle* p = CLR_GetFreeParticle(emitter);
    if (p == NULL) { printf("No particle to spawn\n"); return; }
    p->tick = emitter->def.duration;
    p->pos = pos;
    p->scale = emitter->def.startScale;
    p->velocity = vel;
}

internal void CLR_CullParticles(ZRParticleEmitter* emitter)
{
    // iterate again and cull
    for (i32 i = 0; i < emitter->numParticles; ++i)
    {
        ZRParticle* p = &emitter->particles[i];
        if (p->bCull == NO) { continue; }
        // disable flag
        p->bCull = NO;
        // cull
        i32 lastIndex = emitter->numParticles - 1;
        if (i == 0 && emitter->numParticles == 1)
        {
            // last active particle, just reset list
            emitter->numParticles = 0;
        }
        else if (i == lastIndex)
        {
            // last in array, just truncate
            emitter->numParticles--;
        }
        else
        {
            // swap and truncate
            ZRParticle* last = &emitter->particles[lastIndex];
            *p = *last;
            emitter->numParticles--;
        }
    }
}

internal void CLR_TickParticles(ZRParticleEmitter* emitter, timeFloat delta)
{
    // TODO possible to update and cull in the same iteration?
    // iterate and update
    for (i32 i = 0; i < emitter->numParticles; ++i)
    {
        ZRParticle* p = &emitter->particles[i];
        ///////////////////////////////////////////////
        // Check for recycle
        if (p->tick <= 0)
        {
            // mark for recycle
            p->bCull = YES;
            continue;
        }
        p->tick -= (f32)delta;
        p->prevPos = p->pos;
        Vec3 pull;
        pull.x = emitter->def.pull.x * (f32)delta;
        pull.y = emitter->def.pull.y * (f32)delta;
        pull.z = emitter->def.pull.z * (f32)delta;
        p->velocity.x += pull.x;
        p->velocity.y += pull.y;
        p->velocity.z += pull.z;
        p->pos.x += p->velocity.x * (f32)delta;
        p->pos.y += p->velocity.y * (f32)delta;
        p->pos.z += p->velocity.z * (f32)delta;
    }
    CLR_CullParticles(emitter);
}

extern "C" void CLR_TickTestParticles(timeFloat delta)
{
    CLR_TickParticles(&g_testEmit, delta);
    CLR_TickParticles(&g_gibEmit, delta);
}

internal void CLR_WriteParticles(
    ZRParticleEmitter* emitter, ZRSceneFrame* scene, ZEByteBuffer* list, ZEByteBuffer* data)
{
    ZRMaterial* mat;
    mat = g_assetDb->GetMaterialByIndex(g_assetDb, emitter->def.materialIndex);
    for (i32 i = 0; i < emitter->numParticles; ++i)
    {
        ZRParticle* p = &emitter->particles[i];
        ZRDrawObj* obj = ZRDrawObj_InitInPlace(&list->cursor);
        scene->params.numObjects++;
        obj->data.SetAsMesh(emitter->def.meshIndex, emitter->def.materialIndex);
        obj->data.model.billboard = emitter->def.billboard;
        obj->t.pos = p->pos;
        obj->t.scale = p->scale;
        obj->prevPos = p->prevPos;
    }
}

internal void CLR_AddTestParticles(ZRSceneFrame* scene, ZEByteBuffer* list, ZEByteBuffer* data)
{
    CLR_WriteParticles(&g_testEmit, scene, list, data);
    CLR_WriteParticles(&g_gibEmit, scene, list, data);
}

// Returns number of objects added
internal i32 CLR_Debug_AddAABB(ZEByteBuffer* list, i32 factoryType, Vec3 pos, Vec3 scale)
{
    Vec3 half = scale;
    half.x *= 0.5f;
    half.y *= 0.5f;
    half.z *= 0.5f;
    ZRDrawObj* obj;
    i32 objCount = 0;
    if (factoryType == SIM_FACTORY_TYPE_ACTOR)
    {
        // turn player AABB into dots on the floor
        pos.y -= half.y;
        scale.y = 0.2f;
    }
    // vertical lines
    obj = ZRDrawObj_InitInPlace(&list->cursor);
    obj->data.SetAsMesh(0, 0);
    obj->t.pos = { pos.x + half.x, pos.y, pos.z + half.z };
    obj->t.scale = { 0.1f, scale.y, 0.1f };
    objCount++;

    obj = ZRDrawObj_InitInPlace(&list->cursor);
    obj->data.SetAsMesh(0, 0);
    obj->t.pos = { pos.x - half.x, pos.y, pos.z + half.z };
    obj->t.scale = { 0.1f, scale.y, 0.1f };
    objCount++;

    obj = ZRDrawObj_InitInPlace(&list->cursor);
    obj->data.SetAsMesh(0, 0);
    obj->t.pos = { pos.x + half.x, pos.y, pos.z - half.z };
    obj->t.scale = { 0.1f, scale.y, 0.1f };
    objCount++;

    obj = ZRDrawObj_InitInPlace(&list->cursor);
    obj->data.SetAsMesh(0, 0);
    obj->t.pos = { pos.x - half.x, pos.y, pos.z - half.z };
    obj->t.scale = { 0.1f, scale.y, 0.1f };
    objCount++;
    return objCount;
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
    i32 defaultMaterialIndex = ZRDB_GET_MAT_BY_NAME(g_assetDb, ZRDB_MAT_NAME_WORLD_DEBUG)->index;
    i32 prjMaterialIndex = ZRDB_GET_MAT_BY_NAME(g_assetDb, ZRDB_MAT_NAME_PRJ)->index;

    i32 materialIndex = defaultMaterialIndex;

    i32 objCount = 0;
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE) { continue; }
        materialIndex = defaultMaterialIndex;
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
            case SIM_FACTORY_TYPE_RUBBLE:
            case SIM_FACTORY_TYPE_ACTOR:
            case SIM_FACTORY_TYPE_EXPLOSION:
            case SIM_FACTORY_TYPE_BULLET_IMPACT:
            //case SIM_FACTORY_TYPE_WORLD:
            {
                objCount += CLR_Debug_AddAABB(list, ent->factoryType, ent->body.t.pos, ent->body.t.scale);
                // ZRDrawObj* obj = ZRDrawObj_InitInPlace(&list->cursor);
                // obj->data.SetAsMesh(cubeIndex, materialIndex);
                // obj->t = ent->body.t;
                // objCount++;
            } break;
        }
    }
    return objCount;
}

/**
 * returns number of objects added
 */
internal i32 CLR_AddSimObjectsToRenderScene(
    SimScene* sim,
    ZEByteBuffer* list,
    ZEByteBuffer* scratch,
    ClientRenderSettings cfg)
{
    ZRAssetDB* db = App_GetAssetDB();
    i32 lightBulbMatIndex = ZRDB_GET_MAT_BY_NAME(db, ZRDB_MAT_NAME_LIGHT)->index;
    
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
                obj->prevPos = ent->body.previousPos;
                rendObjectsAdded++;

                // add an optional light source
                if (ent->lightType == 1 && extraLights > 0)
                {
                    obj = ZRDrawObj_InitInPlace(&list->cursor);
                    obj->data.SetAsPointLight({ 1, 1, 0.5f }, 1, 5);
                    obj->t = ent->body.t;
                    obj->prevPos = ent->body.previousPos;
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
                    if (ent->lightType == 1)
                    {
                        obj = ZRDrawObj_InitInPlace(&list->cursor);
                        obj->data.SetAsMesh(0, lightBulbMatIndex);
                        obj->t = ent->body.t;
                        rendObjectsAdded++;
                    }
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
    frame->timestamp = sim->time;
    
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
    // else
    // {
        objCount += CLR_AddSimObjectsToRenderScene(sim, list, data, cfg);
    // }

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
        i32 wallMesh = ZRDB_GET_MESH_BY_NAME(g_assetDb, ZRDB_MAT_NAME_WORLD)->header.index;
        i32 wallMat = ZRDB_GET_MAT_BY_NAME(g_assetDb, ZRDB_MAT_NAME_WORLD)->index;
        #if 1 // right hand
        scene = ZRSccene_InitInPlace(frame->list, ZR_PROJECTION_MODE_3D, NO);
        Transform_SetToIdentity(&scene->params.camera);
        frame->numScenes++;
        obj = ZRDrawObj_InitInPlace(&list->cursor);
        obj->data.SetAsMesh(wallMesh, wallMat);
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
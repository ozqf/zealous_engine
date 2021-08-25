#include "../../headers/zengine.h"

// for rand()
#include <stdlib.h>

#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"
#define MOVE_SPEED 4

#define SHOOT_LEFT "shoot_left"
#define SHOOT_RIGHT "shoot_right"
#define SHOOT_UP "shoot_up"
#define SHOOT_DOWN "shoot_down"

#define ENT_FLAG_COLLISION_SOLID (1 << 0)

#define MAX_ENTITIES 1024

#define ENT_TYPE_NONE 0
#define ENT_TYPE_PLAYER 1
#define ENT_TYPE_PLAYER_PROJECTILE 2
#define ENT_TYPE_ENEMY 3
#define ENT_TYPE_SPAWNER 4

#define ENT_STATE_NONE 0
#define ENT_STATE_DEAD 1 // do not interact with anything that is dead!

struct Entity;

typedef void(EntityTick)(Entity* ent, f32 delta);

struct Entity
{
    i32 id;
    i32 type;
    i32 state;
    f32 tick;
    i32 flags;
    Transform t;
    Vec3 velocity;
    AABB aabb;
    zeHandle drawObj;
    EntityTick* tickFunction;
};

internal ZEngine g_engine;
internal zeHandle g_gameScene = 0;
internal zeHandle g_avatarId = 0;
internal Transform g_camera;

internal ZEBlobStore g_entities;
internal i32 g_nextEntId = 1;
internal AABB g_arenaBounds = {};
internal f32 g_time = 0;

// display settings for each entity type
internal ZRMeshObjData g_playerMeshObjData;
internal ZRMeshObjData g_wallMeshObjData;
internal ZRMeshObjData g_projMeshObjData;
internal ZRMeshObjData g_enemyMeshObjData;

// add/remove entity definitions
internal Entity* CreateEntity(i32 entType);
internal Entity* CreatePlayerProjectile(Vec3 pos, Vec3 dir);
internal Entity* CreateEnemy(Vec3 pos, f32 yaw);

// entity update functions
internal void TickPlayer(Entity* ent, f32 delta);
internal void TickProjectile(Entity *ent, f32 delta);
internal void TickEnemy(Entity* ent, f32 delta);

internal Entity* CreateEntity(i32 entType)
{
    i32 newId = g_nextEntId++;
    Entity *ent = (Entity *)g_entities.GetFreeSlot(newId);
    *ent = {};
    ent->type = entType;
    ent->id = newId;
    Transform_SetToIdentity(&ent->t);
    return ent;
}

internal void QueueEntityRemoval(Entity* ent)
{
	ent->state = ENT_STATE_DEAD;
    g_entities.MarkForRemoval(ent->id);
    if (ent->drawObj != 0)
    {
        g_engine.scenes.RemoveObject(g_gameScene, ent->drawObj);
    }
}

internal void AttachModelToEntity(Entity* ent, ZRMeshObjData meshData, Vec3 scale)
{
    ZRDrawObj *drawObj = g_engine.scenes.AddObject(g_gameScene);
    drawObj->data.SetAsMeshFromData(meshData);
    drawObj->t.pos = ent->t.pos;
    if (Vec3_MagnitudeSqr(&scale) == 0)
    {
        drawObj->t.scale = ent->t.scale;
    }
    else
    {
        drawObj->t.scale = scale;
    }
    ent->drawObj = drawObj->id;
}

internal void SyncEntityDrawObj(Entity* ent)
{
    ZRDrawObj *obj = g_engine.scenes.GetObject(g_gameScene, ent->drawObj);
    if (obj == NULL) { return; }
    obj->t.pos = ent->t.pos;
    obj->t.rotation = ent->t.rotation;
}

internal Entity* CreatePlayerProjectile(Vec3 pos, Vec3 dir)
{
    Entity *ent = CreateEntity(ENT_TYPE_PLAYER_PROJECTILE);
    ent->tickFunction = TickProjectile;
    Vec3_SetMagnitude(&dir, 10);
    ent->t.pos = pos;
    ent->velocity = dir;
    ent->t.scale = {0.2f, 0.2f, 0.2f};
    ent->tick = 0;
    ent->flags |= ENT_FLAG_COLLISION_SOLID;
    ent->aabb.min = { -0.25, -0.25, -0.25 };
    ent->aabb.min = { 0.25, 0.25, 0.25 };
    AttachModelToEntity(ent, g_projMeshObjData, { 0.2f, 0.2f, 0.2f });
    return ent;
}

internal Entity* CreatePlayer(Vec3 pos, f32 yaw)
{
    Entity* ent = CreateEntity(ENT_TYPE_PLAYER);
    ent->t.pos = pos;
    ent->tickFunction = TickPlayer;
    ent->t.scale = { 0.5f, 0.5f, 0.5f };
    ent->flags |= ENT_FLAG_COLLISION_SOLID;
    ent->aabb.min = { -0.25f, -0.25f, -0.25f };
    ent->aabb.max = { 0.25f, 0.25f, 0.25f };
    // AttachModelToEntity(ent, g_playerMeshObjData, ent->t.scale);
    ZRDrawObj *drawObj = g_engine.scenes.AddObject(g_gameScene);
    drawObj->data.SetAsBoundingBox(ent->aabb, COLOUR_U32_GREEN);
    SyncEntityDrawObj(ent);
    return ent;
}

internal Entity* CreateEnemy(Vec3 pos, f32 yaw)
{
    Entity* ent = CreateEntity(ENT_TYPE_ENEMY);
    ent->tickFunction = TickEnemy;
    ent->t.pos = pos;
    ent->t.scale = { 0.5f, 0.5f, 0.5f };
    ent->flags |= ENT_FLAG_COLLISION_SOLID;
    ent->aabb.min = { -0.25f, -0.25f, -0.25f };
    ent->aabb.max = { 0.25f, 0.25f, 0.25f };
    AttachModelToEntity(ent, g_enemyMeshObjData, {});
    return ent;
}

internal i32 TouchEntities(Entity* a, Entity* b)
{
    return 0;
}

internal void CheckCollision(
    Entity *subject, i32* resultIds, i32* numResults, i32 maxResults)
{
    for (i32 i = 0; i < g_entities.m_array->m_numBlobs; ++i)
    {
        Entity *ent = (Entity *)g_entities.GetByIndex(i);
        if (ent == NULL) { continue; }
        // don't collide with yourself!
        if (subject->id == ent->id) { continue; }
		// don't interact with dead stuff
		if (ent->state == ENT_STATE_DEAD) { continue; }
        // is target solid?
        if ((ent->flags & ENT_FLAG_COLLISION_SOLID) == 0) { continue; }

        // TODO - remove me - filtering to just enemies
        if (ent->type != ENT_TYPE_ENEMY) { continue; }

        // test
        AABB mobAABB = ZE_CreateWorldAABB(ent->t.pos, &ent->aabb);
        // printf("Player (%.3f, %.3f) vs mob aabb: %.3f,%.3f to %.3f, %.3f\n",
        //     subject->t.pos.x, subject->t.pos.z,
        //     mobAABB.min.x, mobAABB.min.z, mobAABB.max.x, mobAABB.max.z);
        i32 result = ZE_Vec3VsAABB(subject->t.pos, &mobAABB);
        if (!result) { continue; }
        // printf("Player touching enemy\n");
        resultIds[*numResults] = ent->id;
        *numResults += 1;
        if (*numResults >= maxResults) { return; }
    }
}

internal void SpawnTestEnemy()
{
    f32 rX = (f32)rand() / RAND_MAX;
    // f32 rY = (f32)rand() / RAND_MAX;
    f32 rZ = (f32)rand() / RAND_MAX;
    Vec3 p = AABB_RandomInside(g_arenaBounds, rX, 0.5f, rZ);
    printf("Random: %.3f, %.3f, %.3f\n",
        p.x, p.y, p.z);
    CreateEnemy(p, 0);
}

internal void TickEnemy(Entity* ent, f32 delta)
{
    ent->t.pos = ZE_BoundaryPointCheck(ent->t.pos, &g_arenaBounds);
}

internal void TickPlayer(Entity* ent, f32 delta)
{

    Vec3 moveDir = {};
    if (g_engine.input.GetActionValue(MOVE_LEFT))
    {
        moveDir.x -= 1;
    }
    if (g_engine.input.GetActionValue(MOVE_RIGHT))
    {
        moveDir.x += 1;
    }
    if (g_engine.input.GetActionValue(MOVE_UP))
    {
        moveDir.z -= 1;
    }
    if (g_engine.input.GetActionValue(MOVE_DOWN))
    {
        moveDir.z += 1;
    }

    Vec3_Normalise(&moveDir);
    moveDir.x *= MOVE_SPEED * delta;
    moveDir.y *= MOVE_SPEED * delta;
    moveDir.z *= MOVE_SPEED * delta;

    
    ent->t.pos.x += moveDir.x;
    ent->t.pos.y += moveDir.y;
    ent->t.pos.z += moveDir.z;

    ent->t.pos = ZE_BoundaryPointCheck(ent->t.pos, &g_arenaBounds);

    SyncEntityDrawObj(ent);
    
    if (ent->tick > 0)
    {
        ent->tick -= delta;
    }
    
    Vec3 shootDir = {};
    i32 bShoot = NO;
    if (g_engine.input.GetActionValue(SHOOT_LEFT))
    {
        shootDir.x -= 1;
        bShoot = YES;
    }
    if (g_engine.input.GetActionValue(SHOOT_RIGHT))
    {
        shootDir.x += 1;
        bShoot = YES;
    }
    if (g_engine.input.GetActionValue(SHOOT_UP))
    {
        shootDir.z -= 1;
        bShoot = YES;
    }
    if (g_engine.input.GetActionValue(SHOOT_DOWN))
    {
        shootDir.z += 1;
        bShoot = YES;
    }
    if (bShoot == YES && ent->tick <= 0)
    {
        ent->tick = 0.1f;
        Vec3_Normalise(&shootDir);
        CreatePlayerProjectile(ent->t.pos, shootDir);
    }

    const i32 maxTouchResults = 32;
    i32 touchResults[maxTouchResults];
    i32 numTouchResults = 0;

    CheckCollision(ent, touchResults, &numTouchResults, maxTouchResults);
    if (numTouchResults > 0)
    {
        printf("Player touching %d ents\n", numTouchResults);
    }
}

internal void TickProjectile(Entity* ent, f32 delta)
{
    ent->t.pos.x += ent->velocity.x * delta;
    ent->t.pos.y += ent->velocity.y * delta;
    ent->t.pos.z += ent->velocity.z * delta;
    if (ZE_Vec3VsAABB(ent->t.pos, &g_arenaBounds) == NO)
    {
        g_entities.MarkForRemoval(ent->id);
        g_engine.scenes.RemoveObject(g_gameScene, ent->drawObj);
        return;
    }

    ZRDrawObj *drawObj = g_engine.scenes.GetObject(g_gameScene, ent->drawObj);
    drawObj->t = ent->t;
    ent->tick += delta;
    if (ent->tick >= 1)
    {
        g_entities.MarkForRemoval(ent->id);
        g_engine.scenes.RemoveObject(g_gameScene, ent->drawObj);
    }

    const i32 maxTouchResults = 32;
    i32 touchResults[maxTouchResults];
    i32 numTouchResults = 0;

    CheckCollision(ent, touchResults, &numTouchResults, maxTouchResults);
    if (numTouchResults > 0)
    {
        printf("Projectile touching %d ents\n", numTouchResults);
    }
    for (i32 i = 0; i < numTouchResults; ++i)
    {
        printf("Culling ent %d\n", touchResults[i]);
        Entity* victim = (Entity*)g_entities.GetById(touchResults[i]);
        QueueEntityRemoval(victim);
    }
}

// Function callback for our custom console command
ZCMD_CALLBACK(Exec_LoadLevel)
{
    printf("Game - load level\n");
}

//////////////////////////////////////////////////////////////
// Init
//////////////////////////////////////////////////////////////

internal void Init()
{
    // register inputs
    g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
    g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "move_up");
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_down");

    g_engine.input.AddAction(Z_INPUT_CODE_LEFT, Z_INPUT_CODE_NULL, "shoot_left");
    g_engine.input.AddAction(Z_INPUT_CODE_RIGHT, Z_INPUT_CODE_NULL, "shoot_right");
    g_engine.input.AddAction(Z_INPUT_CODE_UP, Z_INPUT_CODE_NULL, "shoot_up");
    g_engine.input.AddAction(Z_INPUT_CODE_DOWN, Z_INPUT_CODE_NULL, "shoot_down");

    g_engine.input.AddAction(Z_INPUT_CODE_R, Z_INPUT_CODE_NULL, "debug_spawn");

    // register custom console command callbacks
    g_engine.textCommands.RegisterCommand("loadlevel", "Load provided game level", Exec_LoadLevel);

    //////////////////////////////////////////////////////////////
    // Create draw scene
    //////////////////////////////////////////////////////////////

    // register a visual scene
    g_gameScene = g_engine.scenes.AddScene(0, 1024);

    // setup camera and projection
    M4x4_CREATE(projection)
    ZE_SetupDefault3DProjection(projection.cells, 16.f / 9.f);
    Transform_SetToIdentity(&g_camera);
    Transform_SetRotationDegrees(&g_camera, -75, 0, 0);
    g_camera.pos.y = 6.f;
    g_camera.pos.z = 2.5f;

    g_engine.scenes.SetCamera(g_gameScene, g_camera);
    g_engine.scenes.SetProjection(g_gameScene, projection);

    //////////////////////////////////////////////////////////////
    // Create assets
    //////////////////////////////////////////////////////////////

    // retrieve embedded cube mesh
    ZRMeshAsset *cubeMesh = g_engine.assets.GetMeshByName(ZE_EMBEDDED_CUBE_NAME);

    // create a texture
    ZRTexture *tex = g_engine.assets.AllocTexture(64, 64, "ship_texture");
    // paint texture
    ZGen_FillTexture(tex, {50, 50, 50, 255});
    ZGen_FillTextureRect(tex, {255, 255, 0, 255}, {0, 0}, {32, 32});
    ZGen_FillTextureRect(tex, {255, 255, 0, 255}, {32, 32}, {32, 32});
    // create material
    ZRMaterial *mat = g_engine.assets.BuildMaterial("ship_material", "ship_texture", NULL);

    g_playerMeshObjData = {};
    g_playerMeshObjData.meshId = cubeMesh->header.id;
    g_playerMeshObjData.materialId = mat->header.id;

    // wall material
    tex = g_engine.assets.AllocTexture(64, 64, "arena_wall");
    ZGen_FillTexture(tex, {255, 255, 0, 255});
    mat = g_engine.assets.BuildMaterial("arena_wall_material", "arena_wall", NULL);

    g_wallMeshObjData = {};
    g_wallMeshObjData.meshId = cubeMesh->header.id;
    g_wallMeshObjData.materialId = mat->header.id;

    // player projectile
    tex = g_engine.assets.AllocTexture(64, 64, "player_projectile");
    ZGen_FillTexture(tex, { 255, 0, 0, 255});
    mat = g_engine.assets.BuildMaterial("player_projectile_material", "player_projectile", NULL);

    g_projMeshObjData = {};
    g_projMeshObjData.meshId = cubeMesh->header.id;
    g_projMeshObjData.materialId = mat->header.id;

    // enemy
    tex = g_engine.assets.AllocTexture(64, 64, "enemy");
    ZGen_FillTexture(tex, { 100, 100, 255 });
    mat = g_engine.assets.BuildMaterial("enemy_material", "enemy", NULL);

    g_enemyMeshObjData = {};
    g_enemyMeshObjData.meshId = cubeMesh->header.id;
    g_enemyMeshObjData.materialId = mat->header.id;

    //////////////////////////////////////////////////////////////
    // Create entity list
    //////////////////////////////////////////////////////////////
    zErrorCode err = ZE_InitBlobStore(g_engine.system.Malloc, &g_entities, MAX_ENTITIES, sizeof(Entity), 0);
    ZE_ASSERT(err == ZE_ERROR_NONE, "error creating object pool")

    //////////////////////////////////////////////////////////////
    // Create player avatar
    //////////////////////////////////////////////////////////////
    CreatePlayer({ -2, 0, -2 }, 0);
    
    //////////////////////////////////////////////////////////////
    // create arena
    //////////////////////////////////////////////////////////////
    f32 arenaWidth = 12;
    f32 arenaHeight = 8;
    f32 arenaHalfWidth = arenaWidth / 2;
    f32 arenaHalfHeight = arenaHeight / 2;
    g_arenaBounds.min = { -(arenaWidth / 2), -(arenaHeight / 2), -(arenaHeight / 2) };
    g_arenaBounds.max = { (arenaWidth / 2), (arenaHeight / 2), (arenaHeight / 2) };

    // floor
    tex = g_engine.assets.AllocTexture(64, 64, "arena_floor");
    ZGen_FillTexture(tex, { 0, 50, 0, 255 });
    mat = g_engine.assets.BuildMaterial("arena_floor_material", "arena_floor", NULL);
    ZRDrawObj* obj = g_engine.scenes.AddObject(g_gameScene);
    // obj->data.SetAsMeshFromData(g_playerMeshObjData);
    obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = { arenaWidth, 0.05f, arenaHeight };

    f32 wallWidth = 0.1f;
    
    // left wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMeshFromData(g_wallMeshObjData);
    obj->t.scale = {wallWidth, wallWidth, arenaHeight};
    obj->t.pos = {-arenaHalfWidth, 0, 0};

    // right wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMeshFromData(g_wallMeshObjData);
    obj->t.scale = {wallWidth, wallWidth, arenaHeight};
    obj->t.pos = {arenaHalfWidth, 0, 0};
    
    // far wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMeshFromData(g_wallMeshObjData);
    obj->t.scale = {arenaWidth, wallWidth, wallWidth};
    obj->t.pos = {0, 0, -arenaHalfHeight};

    // near wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMeshFromData(g_wallMeshObjData);
    obj->t.scale = {arenaWidth, wallWidth, wallWidth};
    obj->t.pos = {0, 0, arenaHalfHeight};

    SpawnTestEnemy();
}

internal void Shutdown()
{
    
}

internal void Tick(ZEFrameTimeInfo timing)
{
    g_time += (f32)timing.interval;
    ZRTexture* tex = g_engine.assets.GetTexByName("arena_floor");
    f32 sinValue = sinf(g_time);
    if (sinValue < 0) { sinValue = -sinValue; }
    u8 lerpColour = (u8)ZE_LerpF32(10, 50, sinValue);
    ZGen_FillTexture(tex, { 0, lerpColour, 0, 255});
    tex->header.bIsDirty = YES;

    if (g_engine.input.GetActionValue("debug_spawn"))
    {
        SpawnTestEnemy();
    }

    f32 delta = (f32)timing.interval;
    for (i32 i = 0; i < g_entities.m_array->m_numBlobs; ++i)
    {
        Entity* ent = (Entity*)g_entities.GetByIndex(i);
        if (ent == NULL) { continue;  }
        if (ent->tickFunction != NULL)
        {
            ent->tickFunction(ent, delta);
        }
    }
    g_entities.Truncate();
}

Z_GAME_WINDOWS_LINK_FUNCTION
{
	// link functions
    g_engine = engineImport;
    gameExport->Init = Init;
    gameExport->Tick = Tick;
    gameExport->Shutdown = Shutdown;
    gameExport->sentinel = ZE_SENTINEL;
	// export app info
    *gameDef = {};
    gameDef->windowTitle = "3D Example";
    gameDef->targetFramerate = 60;
	gameDef->bOverrideEscapeKey = YES;
    return ZE_ERROR_NONE;
}

#include "../../headers/zengine.h"

#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"
#define MOVE_SPEED 4

#define SHOOT_LEFT "shoot_left"
#define SHOOT_RIGHT "shoot_right"
#define SHOOT_UP "shoot_up"
#define SHOOT_DOWN "shoot_down"

#define MAX_ENTITIES 1024

#define ENT_TYPE_NONE 0
#define ENT_TYPE_PLAYER 1
#define ENT_TYPE_PLAYER_PROJECTION 2
#define ENT_TYPE_PROJECTILE 2

struct Entity;

typedef void(EntityTick)(Entity* ent, f32 delta);

struct Entity
{
    i32 id;
    i32 type;
    i32 state;
    f32 tick;
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

internal ZRMeshObjData g_playerMeshObjData;
internal ZRMeshObjData g_wallMeshObjData;
internal ZRMeshObjData g_projMeshObjData;
internal ZRMeshObjData g_enemyMeshObjData;

internal void PlayerShoot(Vec3 pos, Vec3 dir);

internal Entity* CreateEntity()
{
    i32 newId = g_nextEntId++;
    Entity *ent = (Entity *)g_entities.GetFreeSlot(newId);
    ent->id = newId;
    Transform_SetToIdentity(&ent->t);
    return ent;
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

    ent->t.pos = ZE_BoundaryPointCheck(&g_arenaBounds, ent->t.pos);

    ZRDrawObj *obj = g_engine.scenes.GetObject(g_gameScene, ent->drawObj);
    obj->t.pos = ent->t.pos;
    obj->t.rotation = ent->t.rotation;

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
        PlayerShoot(ent->t.pos, shootDir);
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
}

internal void PlayerShoot(Vec3 pos, Vec3 dir)
{
    Entity* ent = CreateEntity();
    ent->tickFunction = TickProjectile;
    Vec3_SetMagnitude(&dir, 10);
    ent->t.pos = pos;
    ent->velocity = dir;
    ent->t.scale = { 0.2f, 0.2f, 0.2f };
    ent->tick = 0;

    ZRDrawObj* drawObj = g_engine.scenes.AddObject(g_gameScene);
    drawObj->data.SetAsMeshFromData(g_projMeshObjData);
    drawObj->t.pos = pos;
    ent->drawObj = drawObj->id;
    // printf("Create player ent %d with draw obj %d\n", player->id, player->drawObj);
}

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

    // create an entity
    i32 newId = g_nextEntId++;
    // Entity* player = (Entity*)g_entities.GetFreeSlot(newId);
    // player->id = newId;
    // g_avatarId = newId;
    Entity *player = CreateEntity();
    Transform_SetToIdentity(&player->t);
    player->tickFunction = TickPlayer;
    player->t.pos.z = -2.f;
    player->t.scale = {0.5f, 0.5f, 0.5f};

    // create render object for player
    ZRDrawObj *avatar = g_engine.scenes.AddObject(g_gameScene);
    avatar->data.SetAsMeshFromData(g_playerMeshObjData);
    avatar->t.scale = {0.5f, 0.5f, 0.5f};
    // link render object to entity
    player->drawObj = avatar->id;
    printf("Create player ent %d with draw obj %d\n", player->id, player->drawObj);

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
    // obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = {wallWidth, wallWidth, arenaHeight};
    obj->t.pos = {-arenaHalfWidth, 0, 0};

    // right wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMeshFromData(g_wallMeshObjData);
    // obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = {wallWidth, wallWidth, arenaHeight};
    obj->t.pos = {arenaHalfWidth, 0, 0};
    
    // far wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMeshFromData(g_wallMeshObjData);
    // obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = {arenaWidth, wallWidth, wallWidth};
    obj->t.pos = {0, 0, -arenaHalfHeight};

    // near wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMeshFromData(g_wallMeshObjData);
    // obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = {arenaWidth, wallWidth, wallWidth};
    obj->t.pos = {0, 0, arenaHalfHeight};
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

    f32 delta = (f32)timing.interval;
    for (i32 i = 0; i < g_entities.m_array->m_numBlobs; ++i)
    {
        Entity* ent = (Entity*)g_entities.GetByIndex(i);
        if (ent->tickFunction != NULL)
        {
            ent->tickFunction(ent, delta);
        }
    }
    g_entities.Truncate();
}

Z_GAME_WINDOWS_LINK_FUNCTION
{
    g_engine = engineImport;
    gameExport->Init = Init;
    gameExport->Tick = Tick;
    gameExport->Shutdown = Shutdown;
    gameExport->sentinel = ZE_SENTINEL;
    return ZE_ERROR_NONE;
}

#include "../../headers/zengine.h"

#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"
#define MOVE_SPEED 4

internal ZEngine g_engine;
internal zeHandle g_gameScene = 0;
internal zeHandle g_avatarId = 0;
internal Transform g_camera;
internal AABB g_arenaBounds = {};
internal f32 g_time = 0;

internal void Init()
{
    // register inputs
    g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
    g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "move_up");
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_down");

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
    // Create player avatar
    //////////////////////////////////////////////////////////////

    // create a texture
    ZRTexture* tex = g_engine.assets.AllocTexture(64, 64, "ship_texture");
    // paint texture
    ZGen_FillTexture(tex, { 50, 50, 50, 255 });
    ZGen_FillTextureRect(tex, { 255, 255, 0, 255 }, { 0, 0 }, { 32, 32 });
    ZGen_FillTextureRect(tex, { 255, 255, 0, 255 }, { 32, 32 }, { 32, 32 });
    // create material
    ZRMaterial* mat = g_engine.assets.BuildMaterial("ship_material", "ship_texture", NULL);

    // retrieve embedded cube mesh
    ZRMeshAsset *cubeMesh = g_engine.assets.GetMeshByName(ZE_EMBEDDED_CUBE_NAME);
    // object to the scene
    ZRDrawObj *avatar = g_engine.scenes.AddObject(g_gameScene);
    g_avatarId = avatar->userTag;
    avatar->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    avatar->t.pos.z = -2.f;
    avatar->t.scale = { 0.5f, 0.5f, 0.5f };

    //////////////////////////////////////////////////////////////
    // create arena
    //////////////////////////////////////////////////////////////
    f32 arenaWidth = 12;
    f32 arenaHeight = 8;
    f32 arenaHalfWidth = arenaWidth / 2;
    f32 arenaHalfHeight = arenaHeight / 2;
    g_arenaBounds.min = { -(arenaWidth / 2), -(arenaHeight / 2) };
    g_arenaBounds.max = { (arenaWidth / 2), (arenaHeight / 2) };

    // floor
    tex = g_engine.assets.AllocTexture(64, 64, "arena_floor");
    ZGen_FillTexture(tex, { 0, 50, 0, 255 });
    mat = g_engine.assets.BuildMaterial("arena_floor_material", "arena_floor", NULL);
    ZRDrawObj* obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = { arenaWidth, 0.05f, arenaHeight };

    // wall material
    tex = g_engine.assets.AllocTexture(64, 64, "arena_wall");
    ZGen_FillTexture(tex, {255, 255, 0, 255});
    mat = g_engine.assets.BuildMaterial("arena_wall_material", "arena_wall", NULL);

    f32 wallWidth = 0.1f;
    
    // left wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = {wallWidth, wallWidth, arenaHeight};
    obj->t.pos = {-arenaHalfWidth, 0, 0};

    // right wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = {wallWidth, wallWidth, arenaHeight};
    obj->t.pos = {arenaHalfWidth, 0, 0};
    
    // far wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    obj->t.scale = {arenaWidth, wallWidth, wallWidth};
    obj->t.pos = {0, 0, -arenaHalfHeight};

    // near wall
    obj = g_engine.scenes.AddObject(g_gameScene);
    obj->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
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
    u8 green = (u8)ZE_LerpF32(20, 100, sinValue);
    printf("Green %d, Sin: %.3f\n", green, sinValue);
    ZGen_FillTexture(tex, { 0, green, 0, 255});
    tex->header.bIsDirty = YES;


    f32 delta = (f32)timing.interval;
    Vec3 dir = {};
    if (g_engine.input.GetActionValue(MOVE_LEFT))
    {
        dir.x -= 1;
    }
    if (g_engine.input.GetActionValue(MOVE_RIGHT))
    {
        dir.x += 1;
    }
    if (g_engine.input.GetActionValue(MOVE_UP))
    {
        dir.z -= 1;
    }
    if (g_engine.input.GetActionValue(MOVE_DOWN))
    {
        dir.z += 1;
    }
    Vec3_Normalise(&dir);
    dir.x *= MOVE_SPEED * delta;
    dir.y *= MOVE_SPEED * delta;
    dir.z *= MOVE_SPEED * delta;

    ZRDrawObj *obj = g_engine.scenes.GetObject(g_gameScene, g_avatarId);
    obj->t.pos.x += dir.x;
    obj->t.pos.y += dir.y;
    obj->t.pos.z += dir.z;
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

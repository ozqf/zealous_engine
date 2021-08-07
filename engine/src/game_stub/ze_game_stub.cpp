/*
Dummy game DLL to run if none is found, and for engine testing
Should only import from public headers, not engine internal to
get an idea of the engine's API
*/
#include "../../../headers/zengine.h"

internal ZEngine g_engine;
internal zeHandle g_gameScene = 0;
internal zeHandle g_hudScene = 0;

internal zeHandle g_avatarId = 0;

internal void Stub_Init()
{
    printf("Stub init\n");
    // register inputs
    g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
    g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "move_up");
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_down");
    // create a visual scene
    g_gameScene = g_engine.scenes.AddScene(0, 8);

    // find some assets to use on our objects
    i32 textureId = g_engine.assets.GetTexByName(
        FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME)->header.id;
    #if 0
    // add an object to the scene
    ZRDrawObj* obj1 = g_engine.scenes.AddObject(g_gameScene);
    // configure object
    f32 scale = 0.25f;
    obj1->data.SetAsText(
        "Game\nScene", textureId, COLOUR_U32_GREEN, COLOUR_U32_EMPTY, 0);
    obj1->t.pos.x = -2;
    obj1->t.scale = { scale, scale, scale };

    // add another object
    ZRDrawObj *obj2 = g_engine.scenes.AddObject(g_gameScene);
    *obj2 = *obj1;
    obj2->t.pos.x = 1;
    obj2->t.scale = { scale, scale, scale };

    // add another object
    ZRDrawObj *obj3 = g_engine.scenes.AddObject(g_gameScene);
    *obj3 = *obj1;
    obj3->t.pos.y = 1;
    obj3->t.scale = { scale, scale, scale };
    #endif
    // add a mesh
    ZRDrawObj* cube = g_engine.scenes.AddObject(g_gameScene);
    g_avatarId = cube->userTag;
    ZRMeshAsset* cubeMesh = g_engine.assets.GetMeshByName(ZE_EMBEDDED_CUBE_NAME);
    printf("Game - assigning mesh Id %d to obj\n", cubeMesh->header.id);
    Transform_SetToIdentity(&cube->t);
    cube->t.scale = { 0.25f, 0.25f, 0.25f };
    cube->t.pos.z = 4;
    if (cubeMesh != NULL)
    {
        cube->data.SetAsMesh(cubeMesh->header.id, 0);
    }
    else
    {
        printf("No default mesh found!\n");
    }

    // set the camera and projection for the scene
    TRANSFORM_CREATE(camera)
    Transform_SetRotationDegrees(&camera, 45.f, 0, 0);
    camera.pos.z = -2.f;

    M4x4_CREATE(projection)
    ZE_SetupDefault3DProjection(projection.cells, 16.f / 9.f);

    g_engine.scenes.SetCamera(g_gameScene, camera);
    g_engine.scenes.SetProjection(g_gameScene, projection);
}

internal void Stub_Shutdown()
{

}

internal void Stub_Tick(ZEFrameTimeInfo timing)
{
    f32 delta = (f32)timing.interval;
    // printf("Game tick interval: %.8f\n", delta);
    if (g_engine.input.GetActionValue("move_left"))
    {
        ZRDrawObj* obj = g_engine.scenes.GetObject(g_gameScene, g_avatarId);
        obj->t.pos.x -= 2.f * delta;
    }
    if (g_engine.input.GetActionValue("move_right"))
    {
        ZRDrawObj *obj = g_engine.scenes.GetObject(g_gameScene, g_avatarId);
        obj->t.pos.x += 2.f * delta;
    }
    if (g_engine.input.GetActionValue("move_up"))
    {
        ZRDrawObj *obj = g_engine.scenes.GetObject(g_gameScene, g_avatarId);
        obj->t.pos.z -= 2.f * delta;
    }
    if (g_engine.input.GetActionValue("move_down"))
    {
        ZRDrawObj *obj = g_engine.scenes.GetObject(g_gameScene, g_avatarId);
        obj->t.pos.z += 2.f * delta;
    }
}

ze_external zErrorCode ZGame_StubLinkup(ZEngine engineImport, ZGame *gameExport, ZGameDef *gameDef)
{
    // grab engine function pointers
    g_engine = engineImport;
    // export game function pointers
    gameExport->Init = Stub_Init;
    gameExport->Shutdown = Stub_Shutdown;
    gameExport->Tick = Stub_Tick;
    gameExport->sentinel = ZE_SENTINEL;
    // export app info
    *gameDef = {};
    gameDef->windowTitle = "Zealous Engine Example";
    gameDef->targetFramerate = 60;
    return ZE_ERROR_NONE;
}

/*
Dummy game DLL to run if none is found, and for engine testing
Should only import from public headers, not engine internal to
get an idea of the engine's API
*/
#include "../../../headers/zengine.h"

internal ZEngine g_engine;
internal zeHandle g_gameScene = 0;
internal zeHandle g_hudScene = 0;

internal void Stub_Init()
{
    printf("Stub init\n");
    // create a visual scene
    g_gameScene = g_engine.scenes.AddScene(0, 8);

    // find some assets to use on our objects
    i32 textureId = g_engine.assets.GetTexByName(
        FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME)->header.id;

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

internal void Stub_Tick()
{

}

ze_external ZGame ZGame_StubLinkup(ZEngine engine)
{
    g_engine = engine;
    ZGame game = {};
    game.Init = Stub_Init;
    game.Shutdown = Stub_Shutdown;
    game.Tick = Stub_Tick;
    game.sentinel = ZE_SENTINEL;
    return game;
}

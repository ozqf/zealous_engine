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

internal Transform g_debugOrigin;
internal Transform g_debugCam;

internal void Stub_Init()
{
    printf("Stub init\n");
    Transform_SetToIdentity(&g_debugCam);
	
	// window is not ready yet, and this call will silently fail...
	g_engine.input.SetCursorLocked(YES);
	
    // register inputs
    g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
    g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "move_forward");
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_backward");

    g_engine.input.AddAction(Z_INPUT_CODE_SPACE, Z_INPUT_CODE_NULL, "move_up");
    g_engine.input.AddAction(Z_INPUT_CODE_LEFT_SHIFT, Z_INPUT_CODE_NULL, "move_down");

    g_engine.input.AddAction(Z_INPUT_CODE_UP, Z_INPUT_CODE_NULL, "look_up");
    g_engine.input.AddAction(Z_INPUT_CODE_DOWN, Z_INPUT_CODE_NULL, "look_down");
    g_engine.input.AddAction(Z_INPUT_CODE_LEFT, Z_INPUT_CODE_NULL, "look_left");
    g_engine.input.AddAction(Z_INPUT_CODE_RIGHT, Z_INPUT_CODE_NULL, "look_right");

	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_MOVE_X, Z_INPUT_CODE_NULL, "mouse_x");
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_MOVE_Y, Z_INPUT_CODE_NULL, "mouse_y");
	
    g_engine.input.AddAction(Z_INPUT_CODE_Q, Z_INPUT_CODE_NULL, "roll_left");
    g_engine.input.AddAction(Z_INPUT_CODE_E, Z_INPUT_CODE_NULL, "roll_right");

    g_engine.input.AddAction(Z_INPUT_CODE_R, Z_INPUT_CODE_NULL, "reset");

    // create a visual scene
    g_gameScene = g_engine.scenes.AddScene(0, 1024, 0);

    // find some assets to use on our objects
    i32 textureId = g_engine.assets.GetTexByName(
        FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME)->header.id;
    #if 1
    // add an object to the scene
    ZRDrawObj* obj1 = g_engine.scenes.AddObject(g_gameScene);
    // configure object
    f32 scale = 0.25f;
    obj1->data.SetAsText(
        "3D World\nText!", textureId, COLOUR_U32_GREEN, COLOUR_U32_EMPTY, 0);
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
    ZRMeshAsset* cubeMesh = g_engine.assets.GetMeshByName(ZE_EMBEDDED_CUBE_NAME);
    ZRMaterial *cubeMat = g_engine.assets.GetMaterialByName(FALLBACK_CHEQUER_MATERIAL);
    ZRDrawObj *cube = g_engine.scenes.AddObject(g_gameScene);
    cube->data.SetAsMesh(cubeMesh->header.id, cubeMat->header.id);
    g_avatarId = cube->id;

    // printf("Game - assigning mesh Id %d to obj\n", cubeMesh->header.id);
    Transform_SetToIdentity(&cube->t);
    cube->t.scale = { 1, 1, 1 };
    cube->t.pos.z = -2;

    ////////////////////////////////////////////////////
    // bound box test object
    ZRDrawObj *aabbObj = g_engine.scenes.AddObject(g_gameScene);
    AABB aabb = {};
    aabb.min = { -1.5, -1.5, -1.5 };
    aabb.max = { 1.5, 1.5, 1.5 };
    aabbObj->data.SetAsBoundingBox(aabb, COLOUR_U32_GREEN);

    ////////////////////////////////////////////////////
    // pillars
    cube = g_engine.scenes.AddObject(g_gameScene);
    cube->data.SetAsMesh(cubeMesh->header.id, cubeMat->header.id);
    cube->t.pos.x = 8;
    cube->t.pos.z = 8;
    cube->t.scale = { 2, 10, 2 };

    cube = g_engine.scenes.AddObject(g_gameScene);
    cube->data.SetAsMesh(cubeMesh->header.id, cubeMat->header.id);
    cube->t.pos.x = -8;
    cube->t.pos.z = 8;
    cube->t.scale = { 2, 10, 2 };

    cube = g_engine.scenes.AddObject(g_gameScene);
    cube->data.SetAsMesh(cubeMesh->header.id, cubeMat->header.id);
    cube->t.pos.x = 8;
    cube->t.pos.z = -8;
    cube->t.scale = { 2, 10, 2 };

    cube = g_engine.scenes.AddObject(g_gameScene);
    cube->data.SetAsMesh(cubeMesh->header.id, cubeMat->header.id);
    cube->t.pos.x = -8;
    cube->t.pos.z = -8;
    cube->t.scale = { 2, 10, 2 };


    ////////////////////////////////////////////////////
    // set the camera and projection for the scene
    TRANSFORM_CREATE(camera)
    // Transform_SetRotationDegrees(&camera, 45.f, 0, 0);
    // camera.pos.z = 4.f;
    g_debugCam = g_engine.scenes.GetCamera(g_gameScene);
    g_debugOrigin = g_debugCam;

    g_engine.scenes.SetProjection3D(g_gameScene, 100.f);
}

internal void Stub_Shutdown()
{

}

internal void Stub_Tick(ZEFrameTimeInfo timing)
{
	// g_engine.input.SetCursorLocked(YES);
    f32 delta = (f32)timing.interval;
    // printf("Game tick interval: %.8f\n", delta);
    i32 bMoved = NO;
    Vec3 move = {};
    f32 speed = 3;
    if (g_engine.input.GetActionValue("reset"))
    {
        g_debugCam = g_debugOrigin;
        bMoved = YES;
    }

    if (g_engine.input.GetActionValue("move_left"))
    {
        move.x -= 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_right"))
    {
        move.x += 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_forward"))
    {
        move.z -= 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_backward"))
    {
        move.z += 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_down"))
    {
        move.y -= 1;
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("move_up"))
    {
        move.y += 1;
        bMoved = YES;
    }
    
    Vec3 result = M3x3_Calculate3DMove(&g_debugCam.rotation, move);
    Vec3_MulFPtr(&result, speed * delta);
    Vec3_AddTo(&g_debugCam.pos, result);
    
    float rotRate = 90.f * DEG2RAD;
    if (g_engine.input.GetActionValue("look_up"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, rotRate * delta, 1, 0, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("look_down"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, -rotRate * delta, 1, 0, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("look_left"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, rotRate * delta, 0, 1, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("look_right"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, -rotRate * delta, 0, 1, 0);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("roll_left"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, rotRate * delta, 0, 0, 1);
        bMoved = YES;
    }
    if (g_engine.input.GetActionValue("roll_right"))
    {
        M3x3_RotateByAxis(g_debugCam.rotation.cells, -rotRate * delta, 0, 0, 1);
        bMoved = YES;
    }

    if (bMoved)
    {
        // printf("Cam pos: %.3f, %.3f, %.3f\n",
        //     g_debugCam.pos.x, g_debugCam.pos.y, g_debugCam.pos.z);
        g_engine.scenes.SetCamera(g_gameScene, g_debugCam);
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

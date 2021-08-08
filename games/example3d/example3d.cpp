#include "../../headers/zengine.h"

internal ZEngine g_engine;
internal zeHandle g_gameScene = 0;
internal zeHandle g_avatarId = 0;
internal Transform g_camera;

internal void Init()
{
    // register inputs
    g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
    g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "move_forward");
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_backward");

    // register a visual scene
    g_gameScene = g_engine.scenes.AddScene(0, 1024);

    // create a texture
    ZRTexture* tex = g_engine.assets.AllocTexture(64, 64, "ship_texture");
    // paint texture
    ZGen_FillTexture(tex, { 50, 50, 50, 255 });
    ZGen_FillTextureRect(tex, { 0, 0, 255, 255 }, { 0, 0 }, { 32, 32 });
    ZGen_FillTextureRect(tex, { 0, 0, 255, 255 }, { 32, 32 }, { 32, 32 });
    

    ZRMaterial* mat = g_engine.assets.BuildMaterial("ship_material", "ship_texture", NULL);

    // retrieve embedded cube mesh
    ZRMeshAsset *cubeMesh = g_engine.assets.GetMeshByName(ZE_EMBEDDED_CUBE_NAME);
    // add an object to the scene
    ZRDrawObj *cube = g_engine.scenes.AddObject(g_gameScene);
    cube->data.SetAsMesh(cubeMesh->header.id, mat->header.id);
    cube->t.pos.z = -2.f;

    Transform_SetToIdentity(&g_camera);
    g_camera.pos.z = 2.f;
    
    M4x4_CREATE(projection)
    ZE_SetupDefault3DProjection(projection.cells, 16.f / 9.f);
    g_engine.scenes.SetCamera(g_gameScene, g_camera);
    g_engine.scenes.SetProjection(g_gameScene, projection);

}

internal void Shutdown()
{

}

internal void Tick(ZEFrameTimeInfo timing)
{

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

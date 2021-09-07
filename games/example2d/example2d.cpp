#include "../../headers/zengine.h"

internal ZEngine g_engine;

internal zeHandle g_scene;

internal void Init()
{
    printf("2D demo init\n");
    g_scene = g_engine.scenes.AddScene(0, 256);

    M4x4_CREATE(prj)
    ZE_SetupOrthoProjection(prj.cells, 4, 16.f / 9.f);
    g_engine.scenes.SetProjection(g_scene, prj);

    ZRDrawObj *obj1 = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, { 1, 1 });
    obj1->t.pos = { 0, 0, -4 };

    ZRDrawObj *obj2 = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, { 1, 1 });
    obj2->t.pos = { -2, -2, -2 };

    ZRDrawObj *obj3 = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, { 1, 1 });
    obj3->t.pos = { 2, 2, -2 };
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

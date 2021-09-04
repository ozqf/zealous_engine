#include "../../headers/zengine.h"

internal ZEngine g_engine;

internal zeHandle g_scene;

internal void Init()
{
    printf("2D demo init\n");
    g_scene = g_engine.scenes.AddScene(0, 256);
    ZRDrawObj* obj = g_engine.scenes.AddObject(g_scene);
    obj->data.type = ZR_DRAWOBJ_TYPE_QUAD;
    obj->data.quad.textureId = g_engine.assets.GetTexByName(FALLBACK_TEXTURE_NAME)->header.id;
    obj->data.quad.uvMin = { 0, 0 };
    obj->data.quad.uvMax = { 1, 1 };
    obj->data.quad.offset = { };
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

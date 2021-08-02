/*
Dummy game DLL to run if none is found.
Should only import from public headers, not engine internal
*/
#include "../../../headers/zengine.h"

internal ZEngine g_engine;

internal void Stub_Init()
{
    printf("Stub init\n");
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

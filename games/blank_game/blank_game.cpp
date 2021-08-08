#include "../../headers/zengine.h"

internal ZEngine g_engine;

internal void Init()
{

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

#include "../../headers/zengine.h"

#include "../../plugins/zt_map_converter.h"


internal ZEngine g_engine;

internal void Init()
{
	// test plugin
    // ZT_MapConvert("foo");
    printf("\n=== Test Map Converter ===\n");
    ZTMapOutput* output;
    ZT_MapConvertTest(&output);
    printf("\n");
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

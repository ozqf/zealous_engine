#include "../../internal_headers/zengine_internal.h"

internal zeHandle g_nextHandle = 1;

ze_external zeHandle ZScene_AddScene(i32 order, i32 capacity)
{
    zeHandle result = g_nextHandle;
    g_nextHandle += 1;
    return result;
}

ze_external void* ZScene_AddObject(zeHandle scene)
{
    return NULL;
}

ze_external void ZScene_RemoveScene(zeHandle handle)
{

}

ze_external ZEBuffer* ZScene_GetRenderFrame()
{
    return NULL;
}

ze_external void ZScene_Init()
{

}

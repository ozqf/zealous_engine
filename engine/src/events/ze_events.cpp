/*
Shared system for engine services to send generic events to each other.
*/
#include "../../internal_headers/zengine_internal.h"

struct EventObserver
{
    ZE_EventCallback* callback;
    i32 mask;
};

internal EventObserver g_observers[1024];
internal i32 g_nextObserver = 0;

ze_external zErrorCode ZE_RegisterListener(ZE_EventCallback callbackFn, i32 mask)
{
    EventObserver* ob = &g_observers[g_nextObserver++];
    ob->callback = callbackFn;
    ob->mask = mask;
    return ZE_ERROR_NONE;
}

ze_external void ZEvents_BroadcastImmediately(i32 code, void* data, size_t dataSize)
{

}

ze_external void ZEvents_RunQueue()
{

}

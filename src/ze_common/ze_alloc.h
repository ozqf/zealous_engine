#ifndef ZE_ALLOC_H
#define ZE_ALLOC_H

#include "ze_common.h"

////////////////////////////////////////////
// Ultra basic tracking of memory allocations
////////////////////////////////////////////
struct MallocItem
{
    void* ptr;
    i32 capacity;
    // group/identify this alloc
    i32 tag;
    // for debugging, what is this memory for?
    char* label;
};

struct MallocList
{
    MallocItem* items;
    i32 next;
    i32 max;
    i32 totalBytes;
};

internal MallocList COM_InitMallocList(MallocItem* items, i32 capacity)
{
    MallocList list = {};
    list.items = items;
    list.next = 0;
    list.max = capacity;
    list.totalBytes = 0;
    return list;
}

internal u32 COM_SumMallocs(MallocList* list)
{
    i32 total = 0;
    for (i32 i = 0; i < list->next; ++i)
    {
        total += list->items[i].capacity;
    }
    list->totalBytes = total;
    return total;
}

internal void* COM_Malloc(MallocList* list, i32 capacity, i32 tag, char* label)
{
    MallocItem* a = &list->items[list->next++];
    a->ptr = malloc(capacity);
    a->tag = tag;
    ZE_ASSERT(a->ptr, "malloc failed");
    a->capacity = capacity;
    a->label = label;
    ZE_SET_ZERO((u8*)a->ptr, capacity);
    list->totalBytes += capacity;
    return a->ptr;
}

internal void COM_FreeAll(MallocList* list)
{
    for (i32 i = 0; i < list->next; ++i)
    {
        free(list->items[i].ptr);
        list->items[i] = {};
    }
    list->next = 0;
}

#if 1
internal MallocItem* COM_SetAlloc(MallocList* list, void* ptr, i32 capacity, char* label)
{
    MallocItem* a = &list->items[list->next++];
    a->ptr = ptr;
    a->capacity = capacity;
    a->label = label;
    return a;
}
#endif

#endif // ZE_ALLOC_H
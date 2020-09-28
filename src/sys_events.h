#ifndef SYS_EVENTS_H
#define SYS_EVENTS_H

/*
Defines event queue from platform resources
(input, packets, OS messages etc) to App modules
*/
#include "ze_common/ze_common.h"
#include "ze_common/ze_net_types.h"
#include "ze_common/ze_byte_buffer.h"

#define SYS_EVENT_SENTINEL 0xDEADBEEF
#define SYS_EVENT_NULL 0
#define SYS_EVENT_SKIP 1
#define SYS_EVENT_INPUT 2
#define SYS_EVENT_PACKET 3

#define SYS_CAST_EVENT_TO_BASE(sysEvPtr) ((SysEvent*)##sysEvPtr##)

// place at top of all event structs
struct SysEvent
{
    i32 sentinel;
    i32 type;
    i32 size;
};

struct SysInputEvent
{
    SysEvent header;
    u32 inputID = 0;
    i32 value = 0;
    f32 normalised = 0;
};

struct SysPacketEvent
{
    SysEvent header;
    i32 socketIndex;
    ZNetAddress sender;
    //number of bytes of data immediately following this struct
    i32 numBytes;
};

static void Sys_PrepareEvent(SysEvent* ev, i32 type, i32 size)
{
    ev->sentinel = SYS_EVENT_SENTINEL;
    ev->type = type;
    ev->size = size;
}

static i32 Sys_ValidateEvent(SysEvent* ev)
{
    if (ev == NULL || ev->type == SYS_EVENT_NULL || ev->size <= 0)
    {
        return ZE_ERROR_NULL_ARGUMENT;
    }
    if (ev->sentinel != SYS_EVENT_SENTINEL)
    {
        return ZE_ERROR_DESERIALISE_FAILED;
    }
    return ZE_ERROR_NONE;
}

static void Sys_EnqueueEvent(ZEBuffer* buf, SysEvent* ev)
{
    ErrorCode err = Sys_ValidateEvent(ev);
    ZE_ASSERT(err == ZE_ERROR_NONE, "Invalid SysEvent")
    ZE_ASSERT(buf->Space() >= ev->size, "No space for SysEvent")
    //printf("SYS Enqueue ev %d size %d\n", ev->type, ev->size);
    buf->cursor += ZE_COPY(ev, buf->cursor, ev->size);
}


////////////////////////////////////////////////////
// Concrete event types
////////////////////////////////////////////////////
static void Sys_CreateInputEvent(SysInputEvent* ev, u32 inputID, i32 value, f32 normalised)
{
    Sys_PrepareEvent(
        SYS_CAST_EVENT_TO_BASE(ev),
        SYS_EVENT_INPUT,
        sizeof(SysInputEvent));
    ev->inputID = inputID;
    ev->value = value;
    ev->normalised = normalised;
}

static void Sys_WriteInputEvent(ZEBuffer* b, u32 inputID, i32 value, f32 normalised)
{
    SysInputEvent ev = {};
    Sys_CreateInputEvent(&ev, inputID, value, normalised);
    Sys_EnqueueEvent(b, SYS_CAST_EVENT_TO_BASE(&ev));
}

////////////////////////////////////////////////////
// Packet

static void Sys_WritePacketEvent(
	ZEBuffer* b, i32 socketIndex, ZNetAddress* addr, u8* data, i32 dataSize)
{
    SysPacketEvent ev = {};
    i32 totalSize = sizeof(SysPacketEvent) + dataSize;
	Sys_PrepareEvent(SYS_CAST_EVENT_TO_BASE(&ev), SYS_EVENT_PACKET, totalSize);
    ev.socketIndex = socketIndex;
    ev.sender = *addr;
    //BUF_COPY(b, &ev, sizeof(SysPacketEvent))
    //BUF_COPY(b, data, dataSize)
    b->cursor += ZE_COPY(&ev, b->cursor, sizeof(SysPacketEvent));
    b->cursor += ZE_COPY(data, b->cursor, dataSize);
}

#endif // SYS_EVENTS_H
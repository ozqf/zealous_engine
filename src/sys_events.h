#pragma once
/*
Defines event queue from platform resources
(input, packets, OS messages etc) to App modules
*/
#include "../common/common.h"

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
        return COM_ERROR_NULL_ARGUMENT;
    }
    return COM_ERROR_NONE;
}

static void Sys_EnqueueEvent(ByteBuffer* buf, SysEvent* ev)
{
    ErrorCode err = Sys_ValidateEvent(ev);
    COM_ASSERT(err == COM_ERROR_NONE, "Invalid SysEvent")
    COM_ASSERT(buf->Space() >= ev->size, "No space for SysEvent")
    //printf("SYS Enqueue ev %d size %d\n", ev->type, ev->size);
    buf->ptrWrite += COM_COPY(ev, buf->ptrWrite, ev->size);
}


////////////////////////////////////////////////////
// Concrete event types
////////////////////////////////////////////////////
static void Sys_CreateInputEvent(SysInputEvent* ev, u32 inputID, i32 value)
{
    Sys_PrepareEvent(
        SYS_CAST_EVENT_TO_BASE(ev),
        SYS_EVENT_INPUT,
        sizeof(SysInputEvent));
    ev->inputID = inputID;
    ev->value = value;
}

static void Sys_WriteInputEvent(ByteBuffer* b, u32 inputID, i32 value)
{
    SysInputEvent ev = {};
    Sys_CreateInputEvent(&ev, inputID, value);
    Sys_EnqueueEvent(b, SYS_CAST_EVENT_TO_BASE(&ev));
}

////////////////////////////////////////////////////
// Packet

static void Sys_WritePacketEvent(
	ByteBuffer* b, i32 socketIndex, ZNetAddress* addr, u8* data, i32 dataSize)
{
    SysPacketEvent ev = {};
    i32 totalSize = sizeof(SysPacketEvent) + dataSize;
	Sys_PrepareEvent(SYS_CAST_EVENT_TO_BASE(&ev), SYS_EVENT_PACKET, totalSize);
    ev.socketIndex = socketIndex;
    ev.sender = *addr;
    //BUF_COPY(b, &ev, sizeof(SysPacketEvent))
    //BUF_COPY(b, data, dataSize)
    b->ptrWrite += COM_COPY(&ev, b->ptrWrite, sizeof(SysPacketEvent));
    b->ptrWrite += COM_COPY(data, b->ptrWrite, dataSize);
}

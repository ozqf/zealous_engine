#ifndef ZE_PLATFORM_EVENTS_H
#define ZE_PLATFORM_EVENTS_H

#define MOUSE_X_INPUT_ID 1
#define MOUSE_Y_INPUT_ID 2

#include "ze_common/ze_common.h"
#include "ze_common/ze_buf_block.h"
#include "ze_common/ze_input.h"

#define ZE_PLATFORM_EVENT_NONE 0
#define ZE_PLATFORM_EVENT_KEY 1
#define ZE_PLATFORM_EVENT_RENDER_STATS 2

#define ZKEYS_EV_TYPE_NULL 0
#define ZKEYS_EV_TYPE_KEY 1

struct ze_key_event
{
    BufferBlock header;
    i32 inputId;
    i32 value;
};

internal void ZKeys_WriteEvent(ZEByteBuffer* buf, zeInputCode inputId, i32 value)
{
    ze_key_event* ev = (ze_key_event*)buf->cursor;
    buf->cursor += sizeof(ze_key_event);
    *ev = {};
    BufBlock_PrepareHeader(
        &ev->header, sizeof(ze_key_event), ZKEYS_EV_TYPE_KEY);
    ev->inputId = inputId;
    ev->value = value;
    /*
    ze_key_event ev = {};
    BufBlock_PrepareHeader(
        &ev.header, sizeof(ze_key_event), ZKEYS_EV_TYPE_KEY);
    ev.inputId = inputId;
    ev.value = value;
    buf->cursor += ZE_COPY_STRUCT(&ev, buf->cursor, ze_key_event);
    */
}


#endif // ZE_PLATFORM_EVENTS_H
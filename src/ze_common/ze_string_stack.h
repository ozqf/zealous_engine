#ifndef ZE_STRING_STACK_H
#define ZE_STRING_STACK_H

#include "ze_common_full.h"
#include "ze_byte_buffer.h"
#include "ze_hash.h"

struct ZEStringStackItem
{
	i32 sentinel;
	i32 offset;
	i32 hash;
	i32 len;
	char* chars;
};

static ZEStringStackItem* ZE_AddStackString(ZEByteBuffer* buf, char* str)
{
    u8* origin = buf->cursor;
    ZE_INIT_PTR_IN_PLACE(item, ZEStringStackItem, buf)
    item->sentinel = ZE_SENTINEL;
    item->offset = buf->cursor - origin;
    item->len = ZE_StrLen(str);
	item->chars = (char*)buf->cursor;
	buf->cursor += ZE_CopyStringLimited(str, item->chars, item->len);
    item->hash = ZE_Hash_djb2_Fixed((u8*)str, item->len);
    return item;
}

#endif // ZE_STRING_STACK_H

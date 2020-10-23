#ifndef ZE_STRING_STACK_H
#define ZE_STRING_STACK_H

#include "ze_common_full.h"

struct ZEIntern
{
   i32 hash;
   i32 len;
   i32 headerOffset;
   i32 charsOffset;
};

static ZEIntern* ZStr_Intern(ZELookupTable* table, ZEBuffer* buf, const char* str)
{
	i32 len = ZStr_Len(str);
	ZE_ASSERT(len + sizeof(ZEIntern) <= (u32)buf->Space(), "No space for string intern")

	i32 hash = ZE_Hash_djb2((u8*)str);
	i32 offset = buf->CursorOffset();
	if (table != NULL)
	{
		i32 curOffset = table->FindData(hash);
		if (curOffset != table->m_invalidDataValue)
		{
			printf("String %s already interned!\n", str);
			ZEIntern* current = (ZEIntern*)buf->GetAtOffset(curOffset);
			return current;
		}
		table->Insert(hash, offset);
	}
	ZE_INIT_PTR_IN_PLACE(header, ZEIntern, buf);
	header->hash = hash;
	header->len = len;
	header->headerOffset = offset;
	header->charsOffset = buf->CursorOffset();

	strcpy_s((char*)buf->cursor, len, str);
	buf->cursor += header->len;
	return header;
}

static ZEIntern* ZStr_FindInternLinear(ZEBuffer* strings, const char* query)
{
    i32 hash = ZE_Hash_djb2((u8*)query);
    u8* read = strings->start;
	u8* end = strings->cursor;
	while (read < end)
	{
		ZEIntern* intern = (ZEIntern*)read;
        if (intern->hash == hash) { return intern; }
		//printf("%d: %s\n", intern->hash, strings->GetAtOffset(intern->charsOffset));
		read += sizeof(ZEIntern) + intern->len;
	}
    return NULL;
}

#endif // ZE_STRING_STACK_H

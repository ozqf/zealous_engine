#ifndef ZE_VARS_H
#define ZE_VARS_H

#include "ze_common.h"
#include "ze_byte_buffer.h"
#include "ze_hash.h"
#include "ze_string_utils.h"
#include "ze_lookup_table.h"
//#include "ze_lookup_string_table.h"

//////////////////////////////////////////
// Data types
//////////////////////////////////////////

#define ZEVAR_TYPE_INT 1
#define ZEVAR_TYPE_FLOAT 1
#define ZEVAR_TYPE_STR 3

struct ZEVarUnion
{
	i32 i;
	f32 f;
	struct
	{
		char* chars;
		i32 len;
	} txt;
};

struct ZEVar
{
	i32 sentinel;
	i32 type;
	// total size of struct, name and data to jump to next.
	i32 size;
	//i32 nameHash;
	char* name;
	i32 nameLength;

	ZEVarUnion data;
};

static ZEVar* ZEVar_InitVar(ZEByteBuffer* b, char* name, i32 type)
{
	// init var
	ZE_INIT_PTR_IN_PLACE(zeVar, ZEVar, b)
	if (zeVar == NULL) { return 0; }

	// copy name string
	i32 len = ZE_StrLen(name);
	b->cursor += ZE_CopyStringLimited(name, (char*)b->cursor, len);
	
	// setup var
	zeVar->sentinel = ZE_SENTINEL;
	zeVar->name = name;
	zeVar->nameLength = len;
	zeVar->type = type;
	zeVar->size = sizeof(ZEVar) + len;
	return zeVar;
}

static u32 ZEVar_AddString(ZEByteBuffer* b, char* name, char* txt)
{
	// prepare var
	ZEVar* v = ZEVar_InitVar(b, name, ZEVAR_TYPE_STR);
	// copy string
	i32 len = ZE_StrLen(txt);
	v->data.txt.chars = (char*)b->cursor;
	v->data.txt.len = len;
	b->cursor += ZE_CopyStringLimited(txt, (char*)b->cursor, len);
	v->size = (b->cursor - (u8*)v);
	return ((u8*)v - b->start);
}

/**
 * return offset to new variable, or 0 if add failed
 */
static u32 ZEVar_AddInt(ZEByteBuffer* b, char* name, i32 value)
{
	ZEVar* v = ZEVar_InitVar(b, name, ZEVAR_TYPE_INT);
	if (v == NULL) { return 0; }
	v->data.i = value;
	return ((u8*)v - b->start);
}

/**
 * returns fail if the var found was not valid
 */
// static i32 ZEVar_GetInt(ZELookupStrTable* table, char* key, i32 fail)
// {
// 	i32 data = table->GetData(key, fail);
// 	if (data == -1) { return fail; }
// 	return data;
// }

struct ZEVarSet
{
	i32 numItems;
	char* name;
	ZELookupTable* table;
	ZEByteBuffer data;

	ZEVar* AddInt(char* varName, i32 i)
	{
		i32 offset = ZEVar_AddInt(&data, varName, i);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		ZEVar* r = (ZEVar*)(data.start + offset);
		return r;
	}

	i32 GetInt(char* varName, i32 fail)
	{
		u32 hash = ZE_Hash_djb2((u8*)varName);
		i32 offset = table->FindData(hash);
		if (offset == ZE_ERROR_BAD_INDEX) { return fail; }
		ZEVar* r = (ZEVar*)(data.start + offset);
		return r->data.i;
	}
};

static ZEVarSet ZEVar_CreateSet(char* setName, i32 numKeys, i32 dataBytes)
{
	ZEVarSet s = {};
	s.table = ZE_LT_Create(numKeys * 2, -1, NULL);
	s.data = Buf_FromMalloc(malloc(dataBytes), dataBytes);
	// store set name in data, before vars
	s.data.cursor += ZE_CopyStringLimited(
		setName, (char*)s.data.cursor, s.data.capacity);
	s.name = (char*)s.data.start;
	return s;
}

#endif // ZE_VARS_H

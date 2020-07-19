#ifndef ZE_VARS_H
#define ZE_VARS_H

#include "ze_common.h"
#include "ze_byte_buffer.h"

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

struct ZEVarSet
{
	i32 numItems;
	char* name;
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
	#if 0
	const u8* writeStart = b->cursor;
	// init var
	ZE_INIT_PTR_IN_PLACE(intVar, ZEVar, b)
	if (intVar == NULL) { return 0; }

	// copy name string
	i32 len = ZE_StrLen(name);
	b->cursor += ZE_CopyStringLimited(name, (char*)b->cursor, len);
	
	// setup var
	intVar->sentinel = ZE_SENTINEL;
	intVar->name = name;
	intVar->nameLength = len;
	intVar->type = ZEVAR_TYPE_INT;
	intVar->size = sizeof(ZEVar) + len;
	intVar->data.i = value;
	
	return (writeStart - b->start);
	#endif
}

/**
 * returns fail if the var found was not valid
 */
static i32 ZEVar_GetInt(ZEByteBuffer* b, i32 offset, i32 fail)
{
	return fail;
}

#endif // ZE_VARS_H

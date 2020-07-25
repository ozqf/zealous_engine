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

#define ZEVAR_MAX_SET_NAME_LENGTH 32

struct ZEVarSet;
static void ZEVar_CheckSize(ZEVarSet* varSet, i32 dataSize);
static void ZEVar_FreeSet(ZEVarSet* varSet);

#define ZEVAR_TYPE_INT 1
#define ZEVAR_TYPE_FLOAT 2
#define ZEVAR_TYPE_STR 3
#define ZEVAR_TYPE_VEC_4 4
#define ZEVAR_TYPE_SET_PTR 5

#define ZEVAR_EMPTY_STRING "\0"

struct ZEVarUnion
{
	i32 i;
	f32 f;
	struct
	{
		char* chars;
		i32 len;
	} txt;
	Vec4 v4;
	ZEVarSet* ptr;
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
	// TODO: if further data is stored (eg a string) this size will
	// have to be updated by the caller! pass in size of additional data?
	zeVar->size = sizeof(ZEVar) + len;
	return zeVar;
}

struct ZEVarSet
{
	ZELookupTable* table;
	ZEByteBuffer data;
	i32 nameLength;
	char name[ZEVAR_MAX_SET_NAME_LENGTH];
	ZEAllocator alloc;
	
	//////////////////////////////////////////
	// Add vars
	//////////////////////////////////////////
	ZEVar* AddInt(char* varName, i32 i)
	{
		i32 size = sizeof(ZEVar) + ZE_StrLen(varName);
		ZEVar_CheckSize(this, size);

		ZEVar* v = ZEVar_InitVar(&data, varName, ZEVAR_TYPE_INT);
		v->data.i = i;

		i32 offset = ((u8*)v - data.start);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		printf("Created int %s: %d\n", v->name, v->data.i);
		return v;
	}

	ZEVar* AddFloat(char* varName, f32 f)
	{
		i32 size = sizeof(ZEVar) + ZE_StrLen(varName);
		ZEVar_CheckSize(this, size);
		
		ZEVar* v = ZEVar_InitVar(&data, varName, ZEVAR_TYPE_FLOAT);
		v->data.f = f;

		i32 offset = ((u8*)v - data.start);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		return v;
	}

	ZEVar* AddSetPointer(char* varName, ZEVarSet* set)
	{
		i32 size = sizeof(ZEVar) + ZE_StrLen(varName);
		ZEVar_CheckSize(this, size);
		
		ZEVar* v = ZEVar_InitVar(&data, varName, ZEVAR_TYPE_SET_PTR);
		v->data.ptr = set;

		i32 offset = ((u8*)v - data.start);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		return v;
	}

	ZEVar* AddVec4(char* varName, Vec4 v4)
	{
		i32 size = sizeof(ZEVar) + ZE_StrLen(varName);
		ZEVar_CheckSize(this, size);
		
		ZEVar* v = ZEVar_InitVar(&data, varName, ZEVAR_TYPE_VEC_4);
		v->data.v4 = v4;

		i32 offset = ((u8*)v - data.start);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		return v;
	}

	ZEVar* AddString(char* varName, char* str)
	{
		// TODO: calculating string length multiple times here
		i32 strLen = ZE_StrLen(str);
		i32 size = sizeof(ZEVar) + strLen +  + ZE_StrLen(varName);
		ZEVar_CheckSize(this, size);
		
		ZEVar* v = ZEVar_InitVar(&data, varName, ZEVAR_TYPE_STR);
		v->data.txt.chars = (char*)(data.cursor);
		v->data.txt.len = strLen;
		data.cursor += ZE_CopyStringLimited(
			str, (char*)data.cursor, strLen);
		v->size = size;

		i32 offset = ((u8*)v - data.start);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		return v;
	}
	
	//////////////////////////////////////////
	// Retrieval
	//////////////////////////////////////////
	ZEVar* GetVar(char* varName, i32 type)
	{
		u32 hash = ZE_Hash_djb2((u8*)varName);
		i32 offset = table->FindData(hash);
		if (offset == ZE_ERROR_BAD_INDEX) { return NULL; }
		ZEVar* v = (ZEVar*)(data.start + offset);
		if (v->type != type) { return NULL; }
		return v;
	}
	
	f32 GetFloat(char* varName, f32 fail)
	{
		ZEVar* v = GetVar(varName, ZEVAR_TYPE_FLOAT);
		if (v == NULL) { return fail; }
		return v->data.f;
	}

	i32 GetInt(char* varName, i32 fail)
	{
		ZEVar* v = GetVar(varName, ZEVAR_TYPE_INT);
		if (v == NULL) { return fail; }
		return v->data.i;
	}

	ZEVarSet* GetVarSet(char* varName)
	{
		ZEVar* v = GetVar(varName, ZEVAR_TYPE_SET_PTR);
		if (v == NULL) { return NULL; }
		return v->data.ptr;
	}

	char* GetString(char* varName)
	{
		ZEVar* v = GetVar(varName, ZEVAR_TYPE_STR);
		if (v == NULL) { return ZEVAR_EMPTY_STRING; }
		return v->data.txt.chars;
	}
	
	//////////////////////////////////////////
	// Rebuild
	//////////////////////////////////////////
	i32 RebuildLookupTable()
	{
		table->Clear();
		u8* read = this->data.start;
		u8* end = this->data.cursor;
		while(read < end)
		{
			ZEVar* v = (ZEVar*)read;
			if (v->sentinel != ZE_SENTINEL)
			{
				printf("\tdesync - read vars failed\n");
				return ZE_ERROR_UNKNOWN;
			}
			i32 offset = read - this->data.start;
			u32 hash = ZE_Hash_djb2((u8*)v->name);
			// step cursor
			read += v->size;
			table->Insert(hash, offset);
		}
		return ZE_ERROR_NONE;
	}
};

//////////////////////////////////////////
// Create/Rebuild
//////////////////////////////////////////

static i32 ZEVar_CopySet(ZEVarSet* source, ZEVarSet* target)
{
	if (source == NULL || target == NULL) { return ZE_ERROR_BAD_ARGUMENT; }
	// TODO: This check does not take into account the set's name
	// at the start of the data buffer!
	if (target->data.capacity < source->data.Written())
	{
		return ZE_ERROR_NO_SPACE;
	}
	// clear data section of target and copy.
	target->data.Clear(NO);
	target->data.cursor += target->nameLength;
	//i32 written = target->data.cursor - target->dataFull.start;
	i32 written = ZE_COPY(source->data.start, target->data.start, source->data.Written());
	target->data.cursor = target->data.start + written;
	
	// build target's lookup table
	ErrorCode err = target->RebuildLookupTable();
	if (err != ZE_ERROR_NONE)
	{
		printf("Failed to generate ZEVarSet lookup table: %d\n", err);
	}
	return ZE_ERROR_NONE;
}

static void ZEVar_CheckSize(ZEVarSet* varSet, i32 dataSize)
{
	i32 maxKeys = varSet->table->m_maxKeys;
	i32 numKeys = varSet->table->m_numKeys;
	i32 bResize = NO;
	if (numKeys > (maxKeys / 2))
	{
		// enlarge keys table
		bResize = YES;
		maxKeys *= 2;
	}

	i32 minimum = varSet->data.Written() + dataSize;
	i32 capacity = varSet->data.capacity;
	if (capacity < minimum)
	{
		// enlarge data space.
		bResize = YES;
		// repeatedly enlarge to make sure new data fits
		while (capacity < minimum)
		{
			capacity *= 2;
		}
	}
	if (!bResize) { return; }

	// Resize time
	// Create new buffers, copy, free old ones, assign new ones.
	ZEByteBuffer oldBuf = varSet->data;
	// huh? Exception thrown at 0x771635D0 (ntdll.dll) in zetools.exe: 0xC0000005 : Access violation reading location 0x0049F13A.
	void* ptr = varSet->alloc.Allocate(capacity);
	varSet->data = Buf_FromMalloc(ptr, capacity);
	varSet->data.cursor += ZE_COPY(oldBuf.start, varSet->data.cursor, oldBuf.Written());
	varSet->alloc.Free(oldBuf.start);
	
	// just trash the key store and rebuild
	varSet->alloc.Free(varSet->table);
	i32 tableBytes = ZE_LT_CalcBytesForTable(maxKeys);
	varSet->table = ZE_LT_Create(maxKeys, -1,
		(u8*)varSet->alloc.Allocate(tableBytes));
	varSet->RebuildLookupTable();
}

/**
 * if passed in set is null a new one will be allocated
 */
static i32 ZEVar_CreateSet(
	ZEVarSet** result,
	ZEAllocator allocator,
	char* setName,
	i32 numKeys,
	i32 dataBytes)
{
	i32 nameLen = ZE_StrLen(setName);
	if (nameLen > ZEVAR_MAX_SET_NAME_LENGTH)
	{
		return ZE_ERROR_STRING_TOO_LONG;
	}
	// setup allocator
	if (allocator.Allocate == NULL || allocator.Free == NULL)
	{
		printf("ZEVARSET - no allocator funcs passed\n");
		allocator.Allocate = malloc;
		allocator.Free = free;
	}
	// allocate set if not provided
	if (*result == NULL)
	{
		*result = (ZEVarSet*)allocator.Allocate(sizeof(ZEVarSet));
	}
	ZEVarSet* s = *result;
	s->alloc = allocator;
	ZE_CopyStringLimited(setName, s->name, nameLen);
	s->nameLength = nameLen;
	// Create lookup table
	i32 tableBytes = ZE_LT_CalcBytesForTable(numKeys);
	s->table = ZE_LT_Create(numKeys, -1,
		(u8*)allocator.Allocate(tableBytes));
	// store set name in data, before vars
	s->data = Buf_FromMalloc(allocator.Allocate(dataBytes), dataBytes);
	return ZE_ERROR_NONE;
}

static void ZEVar_FreeSet(ZEVarSet* varSet)
{
	// Recursively free children!
	#if 0 // TODO: This is really unsafe if two sets point at the same child set.
	u8* read = varSet->data.start;
	u8* end = varSet->data.cursor;
	while (read < end)
	{
		ZEVar* v = (ZEVar*)read;
		read += v->size;
		if (v->type == ZEVAR_TYPE_SET_PTR && v->data.ptr != NULL)
		{
			printf("Freeing child set %s\n", v->name);
			ZEVar_FreeSet(v->data.ptr);
		}
	}
	#endif
	varSet->alloc.Free(varSet->table);
	varSet->alloc.Free(varSet->data.start);
	varSet->alloc.Free(varSet);
}

#endif // ZE_VARS_H

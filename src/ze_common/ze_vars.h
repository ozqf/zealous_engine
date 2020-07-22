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
#define ZEVAR_TYPE_FLOAT 2
#define ZEVAR_TYPE_STR 3

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
 * return offset to new variable, or 0 if add failed
 */
static u32 ZEVar_AddFloat(ZEByteBuffer* b, char* name, f32 value)
{
	ZEVar* v = ZEVar_InitVar(b, name, ZEVAR_TYPE_FLOAT);
	if (v == NULL) { return 0; }
	v->data.f = value;
	return ((u8*)v - b->start);
}

#define ZEVAR_MAX_SET_NAME_LENGTH 32

struct ZEVarSet;
static void ZEVar_CheckSize(ZEVarSet* varSet, i32 dataSize);

struct ZEVarSet
{
	ZELookupTable* table;
	ZEByteBuffer data;
	i32 nameLength;
	char name[ZEVAR_MAX_SET_NAME_LENGTH];
	
	//////////////////////////////////////////
	// Add vars
	//////////////////////////////////////////
	ZEVar* AddInt(char* varName, i32 i)
	{
		i32 size = sizeof(ZEVar);
		ZEVar_CheckSize(this, size);

		i32 offset = ZEVar_AddInt(&data, varName, i);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		ZEVar* v = (ZEVar*)(data.start + offset);
		return v;
	}

	ZEVar* AddFloat(char* varName, f32 f)
	{
		i32 size = sizeof(ZEVar);
		ZEVar_CheckSize(this, size);
		
		i32 offset = ZEVar_AddFloat(&data, varName, f);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		ZEVar* v = (ZEVar*)(data.start + offset);
		return v;
	}

	ZEVar* AddString(char* varName, char* str)
	{
		i32 size = sizeof(ZEVar) + ZE_StrLen(str);
		ZEVar_CheckSize(this, size);
		
		i32 offset = ZEVar_AddString(&data, varName, str);
		u32 hash = ZE_Hash_djb2((u8*)varName);
		table->Insert(hash, offset);
		ZEVar* v = (ZEVar*)(data.start + offset);
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
	printf("Copying set: %d bytes, %d keys\n",
		source->data.Written(),
		source->table->m_numKeys);
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
	printf("Copied %d bytes and %d keys\n",
		target->data.Written(), target->table->m_numKeys);
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
		printf("Enlarging keys array from %d to %d\n",
			maxKeys, maxKeys * 2);
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

		printf("Enlarging data buffer from %dKB to %dKB\n",
			varSet->data.capacity, capacity);
	}
	if (!bResize) { return; }

	// Resize time
	// Create new buffers, copy, free old ones, assign new ones.
	printf("Rebuild data - alloc %d bytes\n", capacity);
	ZEByteBuffer oldBuf = varSet->data;
	// huh? Exception thrown at 0x771635D0 (ntdll.dll) in zetools.exe: 0xC0000005 : Access violation reading location 0x0049F13A.
	void* ptr = malloc(capacity);
	varSet->data = Buf_FromMalloc(ptr, capacity);
	printf("Copy %d bytes\n", oldBuf.Written());
	varSet->data.cursor += ZE_COPY(oldBuf.start, varSet->data.cursor, oldBuf.Written());
	free(oldBuf.start);
	
	// just trash the key store and rebuild
	printf("Rebuild table\n");
	free(varSet->table);
	varSet->table = ZE_LT_Create(maxKeys, -1, NULL);
	varSet->RebuildLookupTable();
}

/**
 * if passed in set is null a new one will be allocated
 */
static i32 ZEVar_CreateSet(ZEVarSet** result, char* setName, i32 numKeys, i32 dataBytes)
{
	i32 nameLen = ZE_StrLen(setName);
	if (nameLen > ZEVAR_MAX_SET_NAME_LENGTH)
	{
		return ZE_ERROR_STRING_TOO_LONG;
	}
	// allocate set if not provided
	if (*result == NULL)
	{
		*result = (ZEVarSet*)malloc(sizeof(ZEVarSet));
	}
	ZEVarSet* s = *result;
	ZE_CopyStringLimited(setName, s->name, nameLen);
	s->nameLength = nameLen;
	// Create lookup table
	s->table = ZE_LT_Create(numKeys * 2, -1, NULL);
	// store set name in data, before vars
	s->data = Buf_FromMalloc(malloc(dataBytes), dataBytes);
	return ZE_ERROR_NONE;
}

static void ZEVar_FreeSet(ZEVarSet* varSet)
{
	free(varSet->table);
	free(varSet->data.start);
	free(varSet);
}

#endif // ZE_VARS_H

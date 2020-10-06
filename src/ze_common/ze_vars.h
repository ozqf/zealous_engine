#ifndef ZE_VARS_H
#define ZE_VARS_H

/////////////////////////////////
// Second iteration - hopefully less crap
/////////////////////////////////

struct ZEIntern
{
   i32 hash;
   i32 len;
   char* chars;
};

/////////////////////////////////
// First iteration - crap
/////////////////////////////////

/*
TODO: This whole implementation is kinda shit isn't it.
Plan to refactor to intern strings for idenfier names and values into
a shared store. Sets will be hash tables into this store.
*/
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
		i32 offset;
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
	i32 nameOffset;
	i32 nameLength;

	ZEVarUnion data;
};

static ZEVar* ZEVar_InitVar(ZEBuffer* b, char* name, i32 type)
{
	// init var
	ZE_INIT_PTR_IN_PLACE(zeVar, ZEVar, b)
	if (zeVar == NULL) { return 0; }

	// copy name string
	i32 len = ZE_StrLen(name);
	i32 nameOffset = b->CursorOffset();
	b->cursor += ZE_CopyStringLimited(name, (char*)b->cursor, len);
	
	// setup var
	zeVar->sentinel = ZE_SENTINEL;
	zeVar->nameOffset = nameOffset;
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
	ZEBuffer data;
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
		printf("ZEVAR Created int %s: %d\n", (char*)(this->data.start + v->nameOffset), v->data.i);
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
		v->data.txt.offset = data.CursorOffset();
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
		return (char*)(this->data.start + v->data.txt.offset);
	}

	char* GetStringFromVar(ZEVar* v)
	{
		return (char*)(this->data.start + v->data.txt.offset);
	}

	char* GetVarName(ZEVar* v)
	{
		return (char*)(this->data.start + v->nameOffset);
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
			u32 hash = ZE_Hash_djb2((u8*)(GetVarName(v)));
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
	// TODO: Use realloc instead of malloc and free

	// Create new buffers, copy, free old ones, assign new ones.
	ZEBuffer oldBuf = varSet->data;
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
		printf("ZEVARSET - allocating new\n");
		*result = (ZEVarSet*)allocator.Allocate(sizeof(ZEVarSet));
	}
	else
	{
		printf("ZEVARSET - Build pre-allocated set\n");
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

/////////////////////////////////////////////////
// Save sets to text buffer
/////////////////////////////////////////////////
static i32 ZEVar_WriteToTextFile(
	FILE* f, ZEVarSet* set)
{
	fprintf_s(f, "map %s\n", set->name);
	u8* read = set->data.start;
	u8* end = set->data.cursor;
	while (read < end)
	{
		ZEVar* v = (ZEVar*)read;
		read += v->size;
		char* varName = set->GetVarName(v);
		switch (v->type)
		{
			case ZEVAR_TYPE_INT:
			fprintf_s(f, "\ti %s %d\n", varName, v->data.i);
			break;
			case ZEVAR_TYPE_FLOAT:
			fprintf_s(f, "\tf %s %d\n", varName, v->data.i);
			break;
			case ZEVAR_TYPE_VEC_4:
			Vec4 v4 = v->data.v4;
			fprintf_s(f, "\tv %s %f %f %f %f\n",
				varName, v4.x, v4.y, v4.z, v4.w);
			break;
			case ZEVAR_TYPE_STR:
			char* str = set->GetStringFromVar(v);
			fprintf_s(f, "\ts %s %s\n", varName, str);
			break;
		}
	}
	fprintf_s(f, "\n");
	
	return ZE_ERROR_NONE;
}

/////////////////////////////////////////////////
// Load from file
/////////////////////////////////////////////////
#define ZEVAR_READ_STATE_NONE 0
#define ZEVAR_READ_STATE_EAT_LINE 1
#define ZEVAR_READ_STATE_TOKEN 2

static i32 ZEVar_ReadFromText(
	char* file, i32 len, ZEAllocator alloc, ZEVarSet** sets, i32* numSets, i32 maxSets)
{
	if (file == NULL)
	{ return ZE_ERROR_NULL_ARGUMENT; }
	if (len <= 0) { return ZE_ERROR_BAD_ARGUMENT; }
	if (alloc.Allocate == NULL || alloc.Free == NULL)
	{ return ZE_ERROR_NULL_ARGUMENT; }

	printf("ZEVar - reading from %d chars\n", len);

	// The way you should probably do it:
	// https://stackoverflow.com/questions/9930758/best-way-to-read-a-parse-data-from-text-file-in-c

	// or perhaps "Handwritten recursive decent parsing"

	// okay lets do some dodgy parsing
	char* cursor = file;
	char* end = cursor + len;
	char* tokenStart = NULL;
	i32 state = ZEVAR_READ_STATE_NONE;
	i32 lineCount = 0;
	*numSets = 0;
	ZEVarSet* set = NULL;
	ZEVar* v = NULL;
	while (cursor < end)
	{

		// tokenise per line... kinda irritating to isolate lines thanks to \r\n
		#if 1
		char* lineStart = cursor;
		char* lineEnd = ZE_FindNewLineOrEnd(cursor, end);
		cursor = ZE_RunToNextLine(cursor, end);
		lineCount++;
		i32 lineLen = (lineEnd - lineStart);

		// tokenise!
		if (lineLen >= 256)
		{
			printf("Line len %d too long, skipping\n", lineLen);
			continue;
		}
		// have to copy into a buffer and stick on the null terminator
		// required by the tokenise function
		char lineBuf[256];
		
		ZE_COPY(lineStart, lineBuf, lineLen);
		lineBuf[lineLen] = '\0';
		char* tokens[32];

		i32 numTokens = ZE_ReadTokens(lineBuf, lineBuf, tokens, 32);
		// find comment tokens and reduce count.
		for (i32 i = 0; i < numTokens; ++i)
		{
			if (tokens[i][0] == '#')
			{ numTokens = i; break; }
		}
		if (numTokens == 0)
		{
			//printf("Line %d has zero tokens\n", lineCount);
			continue;
		}

		// parse tokens
		if (ZE_CompareStrings(tokens[0], "map") == 0)
		{
			// Create set... previous in progress set is now 'closed'
			if (*numSets >= maxSets)
			{
				printf("No more available handles to create set %s\n", tokens[1]);
				return ZE_ERROR_NONE;
			}
			if (set != NULL)
			{
				set = NULL;
			}
			printf("Creating set %s\n", tokens[1]);
			ErrorCode err = ZEVar_CreateSet(&set, alloc, tokens[1], 8, 256);
			sets[*numSets] = set;
			*numSets += 1;
		}
		else if (set == NULL) 
		{
			// we're not currently creating a set so ignore anything else!
			continue;
		}
		else if (ZE_CompareStrings(tokens[0], "i") == 0)
		{
			// create int
			if (numTokens < 3)
			{ printf("Line %d Not enough tokens to declare int", lineCount); continue; }
			char* varName = tokens[1];
			i32 i = ZE_AsciToInt32(tokens[2]);
			v = set->AddInt(varName, i);
		}
		else if (ZE_CompareStrings(tokens[0], "f") == 0)
		{
			// create float
			if (numTokens < 3)
			{ printf("Line %d Not enough tokens to declare float", lineCount); continue; }
			char* varName = tokens[1];
			f32 f = (f32)atof(tokens[2]);
			printf("Create float %s value %.3f\n", varName, f);
			set->AddFloat(varName, f);
		}
		else if (
			ZE_CompareStrings(tokens[0], "t") == 0
			|| ZE_CompareStrings(tokens[0], "s") == 0)
		{
			// create string
			if (numTokens < 3)
			{ printf("Line %d Not enough tokens to declare string", lineCount); continue; }

			// TODO: Only third token can be included in string
			v = set->AddString(tokens[1], tokens[2]);
			printf("Created Str %s: \"%s\"\n", set->GetVarName(v), set->GetStringFromVar(v));
		}
		else if (ZE_CompareStrings(tokens[0], "v") == 0)
		{
			// create vector
			if (numTokens < 3)
			{ printf("Line %d Not enough tokens to declare vector", lineCount); continue; }
			char* varName = tokens[1];
			Vec4 v4 = {};
			// vector fields are optional.
			// x (token 2)
			if (numTokens >= 3) { v4.x = (f32)atof(tokens[2]); }
			// y (token 3)
			if (numTokens >= 4) { v4.y = (f32)atof(tokens[3]); }
			// z (token 4)
			if (numTokens >= 5) { v4.z = (f32)atof(tokens[4]); }
			// w (token 5)
			if (numTokens >= 6) { v4.w = (f32)atof(tokens[5]); }
			printf("Create v4 %s value %.3f, %.3f, %.3f, %.3f\n",
				varName, v4.x, v4.y, v4.z, v4.w);
			set->AddVec4(varName, v4);
		}
		else
		{
			printf("Unrecognised starting token \"%s\"\n", tokens[0]);
		}
		

		//////////////////////////////////////////
		// print
		/*printf("Line %d (%d chars) - %d tokens\n",
			lineCount, lineLen, numTokens);
		for (i32 i = 0; i < numTokens; ++i)
		{
			printf("%s, ", tokens[i]);
		}
		printf("\n");*/

		#endif
		// ignore tokenise function and just parse char by char
		#if 0
		char c = *cursor;
		cursor++;
		switch (state)
		{
			case ZEVAR_READ_STATE_NONE:
			if (c == '#')
			{
				state = ZEVAR_READ_STATE_EAT_LINE;
			}
			else if (c == ' ' || c == '\t')
			{
				continue;
			}
			else
			{
				// start token
				state = ZEVAR_READ_STATE_TOKEN;
				tokenStart = cursor;
				//printf("%c", c);
			}
			break;
			case ZEVAR_READ_STATE_TOKEN:
			{
				if (c == ' ' || c == '\t')
				{
					// end token
				}
			} break;
			case ZEVAR_READ_STATE_EAT_LINE:
			if (c == '\n') { state = ZEVAR_READ_STATE_NONE; }
			break;
			default:
			printf("%c", c);
			break;
		}
		#endif
	}
	printf("\n\tDone - read %d lines\n", lineCount);
	return ZE_ERROR_NONE;
}

#endif // ZE_VARS_H

#include "../../ze_common/ze_vars.h"

static void ZEVar_List(ZEVarSet* varSet)
{
	printf("=== List ZEVars in set %s ===\n", varSet->name);
	printf("--- Var data ---\n");
	ZEByteBuffer* b = &varSet->data;
	u8* read = b->start;
	// name is stored at the front of the array so
	// advance until passed it
	//read = (u8*)ZE_ReadToNullChar((char*)read);
	//read += skipBytes;
	u8* end = b->cursor;
	i32 numVars = 0;
	i32 totalSize = 0;
	while (read < end)
	{
		ZEVar* v = (ZEVar*)read;
		if (v->sentinel != ZE_SENTINEL)
		{ printf("\tdesync - read vars failed\n"); return; }
		read += v->size;
		totalSize += v->size;
		numVars++;
		i32 offset = (i32)((u8*)v - b->start);
		switch(v->type)
		{
			case ZEVAR_TYPE_INT:
			printf("%s: %d. Int Offset %d\n",
				v->name, v->data.i, offset);
			break;
			case ZEVAR_TYPE_FLOAT:
			printf("%s: %f. float Offset %d\n",
				v->name, v->data.f, offset);
			break;
			case ZEVAR_TYPE_STR:
			printf("%s: \"%s\". Len (%d) Offset %d\n",
				v->name, v->data.txt.chars, v->data.txt.len, offset);
			break;
			case ZEVAR_TYPE_VEC_4:
			Vec4 v4 = v->data.v4;
			printf("%s: %.3f, %.3f, %.3f, %.3f. Vec4 Offset %d\n",
				v->name,
				v4.x, v4.y, v4.z, v4.w,
				offset);
			break;
			case ZEVAR_TYPE_SET_PTR:
			{
				ZEVarSet* s = v->data.ptr;
				if (s == NULL)
				{
					printf("%s: NULL set ptr\n", v->name);
				}
				else
				{
					printf("%s: Set with %d bytes and %d keys\n",
						v->name, s->data.Written(), s->table->m_numKeys);
				}
			}
			break;
			default:
			printf("Unknown var type %d\n", v->type);
			break;
		}
	}
	printf("\t%d vars, avg size %.3f\n", numVars, (f32)totalSize / (f32)numVars);
	
	printf("--- keys ---\n");
	ZELookupTable* t = varSet->table;
	for (i32 i = 0; i < t->m_maxKeys; ++i)
	{
		if (t->m_keys[i].idHash == 0) { continue; }
		printf("%d: hash %d data %d\n", i, t->m_keys[i].idHash, t->m_keys[i].data);
	}
	printf("\n");
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

	// okay lets do some dodgy parsing
	char* cursor = file;
	char* end = cursor + len;
	char* tokenStart = NULL;
	i32 state = ZEVAR_READ_STATE_NONE;
	i32 lineCount = 0;
	*numSets = 0;
	ZEVarSet* set = NULL;
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
				printf("Finished set:\n");
				ZEVar_List(set);
			}
			printf("Creating set %s\n", tokens[1]);
			ErrorCode err = ZEVar_CreateSet(&set, alloc, tokens[1], 8, 256);
			printf("Set pointer %d\n", *numSets);
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
			printf("Create int %s value %d\n", varName, i);
			set->AddInt(varName, i);
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
		else if (ZE_CompareStrings(tokens[0], "t") == 0)
		{
			// create string
			if (numTokens < 3)
			{ printf("Line %d Not enough tokens to declare string", lineCount); continue; }
		}
		else if (ZE_CompareStrings(tokens[0], "v") == 0)
		{
			// create vector
			if (numTokens < 3)
			{ printf("Line %d Not enough tokens to declare vector", lineCount); continue; }
			char* varName = tokens[1];
			Vec4 v = {};
			// vector fields are optional.
			// x (token 2)
			if (numTokens >= 3) { v.x = (f32)atof(tokens[2]); }
			// y (token 3)
			if (numTokens >= 4) { v.y = (f32)atof(tokens[3]); }
			// z (token 4)
			if (numTokens >= 5) { v.z = (f32)atof(tokens[4]); }
			// w (token 5)
			if (numTokens >= 6) { v.w = (f32)atof(tokens[5]); }
			printf("Create v4 %s value %.3f, %.3f, %.3f, %.3f\n",
				varName, v.x, v.y, v.z, v.w);
			set->AddVec4(varName, v);
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
	if (set != NULL)
	{
		printf("Finished set:\n");
		ZEVar_List(set);
	}
	printf("\n\tDone - read %d lines\n", lineCount);
	return ZE_ERROR_NONE;
}


static void Test_ZEVars_Version1()
{
	#if 0
	/*
	> required:
		> a buffer to hold the variables themselves.
		> a buffer to hold the names of said variables.
	*/
	ZEByteBuffer buf = Buf_FromMalloc(malloc(MegaBytes(1)), MegaBytes(1));
	// Create lookup table at the front of the memory block.
	// Store added variables after table.
	i32 maxKeys = 16;
	printf("Bytes for table with %d (%d scaled up) for hash table\n",
		sizeof(ZELookupStrKey) * maxKeys,
		sizeof(ZELookupStrKey) * (maxKeys * 2));
	i32 tableBytes = ZE_MeasureStringHashTable(maxKeys);
	ZELookupStrTable* table = ZE_CreateStringHashTable(maxKeys, buf.start);
	buf.cursor = buf.start + tableBytes;
	// setup buffer inside var list
	table->data = Buf_FromBytes(buf.cursor, buf.Space());

	// test. create key in buffer and record key
	i32 c = ZEVar_AddString(&table->data, "mob_class", "base_goblin");
	table->Insert("mob_class", c);
	i32 a = ZEVar_AddInt(&table->data, "mob_health", 100);
	table->Insert("mob_health", a);
	i32 b = ZEVar_AddInt(&table->data, "mob_run_speed", 5);
	table->Insert("mob_run_speed", b);
	ZEVar_List(&table->data);

	// Recall something
	char* queryKey = "mob_health";
	i32 offset = table->GetData(queryKey, -1);
	if (offset == -1)
	{
		printf("Failed to find key %s\n", queryKey);
		free(buf.start);
		return;
	}
	printf("Offset for %s: %d\n", queryKey, offset);
	ZEVar* v = (ZEVar*)(table->data.start + offset);
	printf("Key %s: %d\n", v->name, v->data.i);

	free(buf.start);
	#endif
}

static i32 g_allocCount = 0;

static void* Test_ZEVarMalloc(size_t numBytes)
{
	void* ptr = malloc(numBytes);
	g_allocCount++;
	printf(">>>>>>>>>>>> Allocated %d at %d - total %d\n",
		numBytes, (u32)ptr, g_allocCount);
	return ptr;
}

static void Test_ZEVarFree(void* ptr)
{
	--g_allocCount;
	printf(">>>>>>>>>>>> Freed memory at %d - total %d\n",
		(u32)ptr, g_allocCount);
	free(ptr);
}

// #define ZV_MAX_MAPS 64
// static ZELookupTable* g_mapTable = NULL;
// static ZEVarSet* g_maps[ZV_MAX_MAPS];
static ZEVarSet* g_maps = NULL;

// example fields for mob definition
#define ZV_MOB_CLASS "mob_class"
#define ZV_MOB_HEALTH "mob_health"
#define ZV_MOB_DAMAGE "mob_damage"
#define ZV_MOB_SPEED "mob_speed"
#define ZV_MOB_STUN_HEALTH "mob_stun_health"
#define ZV_MOB_STUNTIME "mob_stun_time"
#define ZV_MOB_MOVE_TYPE "mob_move_type"
#define ZV_MOB_BODY_SIZE "mob_body_size"

// static void Test_ZEVars_CreateDb()
// {
// 	g_mapTable = ZE_LT_Create(ZV_MAX_MAPS * 2, -1, NULL);
// 	ZE_SET_ZERO(g_maps, sizeof(ZEVarSet*) * ZV_MAX_MAPS);
// }

static void Test_ZEVars_Version2()
{
	ZEAllocator alloc = { Test_ZEVarMalloc, Test_ZEVarFree };
	char* path = "stats.txt";
	printf("Reading stats from file %s\n", path);
	FILE* f = NULL;
	errno_t err = fopen_s(&f, path, "rb");
	if (f == NULL)
	{
		printf("Couldn't open stat file %s\n", path);
		return;
	}
	fseek(f, 0, SEEK_END);
	i32 strLen = ftell(f);
	fseek(f, 0, SEEK_SET);
	void* mem = alloc.Allocate(strLen);
	if (mem == NULL)
	{
		printf("Failed to alloc %d bytes for stat file %s\n", strLen, path);
		fclose(f);
		return;
	}
	fread((void*)mem, strLen, 1, f);
	fclose(f);

	const i32 maxSets = 32;
	ZEVarSet* sets[maxSets];
	i32 numSets = 0;

	ZEVar_ReadFromText((char*)mem, strLen, alloc, sets, &numSets, maxSets);
	printf("---------------------------\n");
	printf("Create %d sets\n", numSets);
	for (i32 i = 0; i < numSets; ++i)
	{
		ZEVar_List(sets[i]);
	}
	alloc.Free(mem);
}



static void Test_ZEVars_Version2_b()
{
	printf("--- ZE Set Version 2 ---\n");

	ZEVarSet* readTest = NULL;
	ZEAllocator alloc = { Test_ZEVarMalloc, Test_ZEVarFree };

	// store sets
	ZEVar_CreateSet(&g_maps, alloc, "maps", 16, 512);

	// create sets
	ZEVarSet* goblin = NULL;
	ZEVar_CreateSet(&goblin, alloc, "goblin", 4, 128);

	g_maps->AddSetPointer(goblin->name, goblin);

	goblin->AddInt(ZV_MOB_HEALTH, 100);
	goblin->AddString(ZV_MOB_CLASS, "class_goblin");
	goblin->AddFloat(ZV_MOB_SPEED, 5.5f);
	goblin->AddInt(ZV_MOB_STUN_HEALTH, 1);
	
	// Clone set:
	printf("- Created cloned set -\n");
	ZEVarSet* clone = NULL;
	ZEVar_CreateSet(
		&clone,
		alloc,
		"goblin_clone",
		goblin->table->m_maxKeys,
		goblin->data.capacity);
	g_maps->AddSetPointer(clone->name, clone);
	printf("Cloning\n");
	ErrorCode err = ZEVar_CopySet(goblin, clone);
	if (err != ZE_ERROR_NONE)
	{
		printf("Error %d copying set\n", err);
		ZEVar_FreeSet(goblin);
		ZEVar_FreeSet(clone);
		return;
	}

	printf(">> Add additional fields to clone\n");
	clone->AddFloat(ZV_MOB_STUNTIME, 2.f);
	clone->AddVec4(ZV_MOB_BODY_SIZE, { 1.2f, 1.9f, 1.2f, 1 });
	clone->AddString(ZV_MOB_MOVE_TYPE, "walk");
	clone->AddInt(ZV_MOB_DAMAGE, 5);
	
	
	readTest = clone;
	
	ZEVar_List(readTest);
	
	printf("%s: %d\n", ZV_MOB_HEALTH, readTest->GetInt(ZV_MOB_HEALTH, 0));
	printf("%s: %d\n", ZV_MOB_DAMAGE, readTest->GetInt(ZV_MOB_DAMAGE, 0));
	printf("%s: %s\n", ZV_MOB_CLASS, readTest->GetString(ZV_MOB_CLASS));
	printf("%s: %.3f\n", ZV_MOB_SPEED, readTest->GetFloat(ZV_MOB_SPEED, 0));
	printf("Set %s: %d of %d keys, %d of %d bytes\n",
		readTest->name,
		readTest->table->m_numKeys, readTest->table->m_maxKeys,
		readTest->data.Written(), readTest->data.capacity);
	/////////////////////////////////////////////////
	
	ZEVar_List(g_maps);
	
	// Cleanup
	ZEVar_FreeSet(goblin);
	ZEVar_FreeSet(clone);
	ZEVar_FreeSet(g_maps);
}

static void Test_ZEVars()
{
	printf("\n=== Test ZEVars ===\n");
	//Test_ZEVars_Version1();
	Test_ZEVars_Version2();
}

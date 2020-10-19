#include "../../ze_common/ze_vars.h"

static void ZEVar_List(ZEVarSet* varSet)
{
	printf("=== List ZEVars in set %s ===\n", varSet->name);
	#if 1
	printf("--- Var data ---\n");
	ZEBuffer* b = &varSet->data;
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
				varSet->GetVarName(v), v->data.i, offset);
			break;
			case ZEVAR_TYPE_FLOAT:
			printf("%s: %f. float Offset %d\n",
				varSet->GetVarName(v), v->data.f, offset);
			break;
			case ZEVAR_TYPE_STR:
			printf("%s: \"%s\". Len (%d) Offset %d\n",
				varSet->GetVarName(v), varSet->GetStringFromVar(v), v->data.txt.len, offset);
			break;
			case ZEVAR_TYPE_VEC_4:
			Vec4 v4 = v->data.v4;
			printf("%s: %.3f, %.3f, %.3f, %.3f. Vec4 Offset %d\n",
				varSet->GetVarName(v),
				v4.x, v4.y, v4.z, v4.w,
				offset);
			break;
			case ZEVAR_TYPE_SET_PTR:
			{
				ZEVarSet* s = v->data.ptr;
				if (s == NULL)
				{
					printf("%s: NULL set ptr\n", varSet->GetVarName(v));
				}
				else
				{
					printf("%s: Set with %d bytes and %d keys\n",
						varSet->GetVarName(v), s->data.Written(), s->table->m_numKeys);
				}
			}
			break;
			default:
			printf("!!Unknown var type %d\n", v->type);
			break;
		}
	}
	printf("\t%d vars, avg size %.3f\n", numVars, (f32)totalSize / (f32)numVars);
	#endif
	#if 0
	printf("--- keys ---\n");
	ZELookupTable* t = varSet->table;
	for (i32 i = 0; i < t->m_maxKeys; ++i)
	{
		if (t->m_keys[i].idHash == 0) { continue; }
		printf("%d: hash %d data %d\n", i, t->m_keys[i].idHash, t->m_keys[i].data);
	}
	#endif
	printf("\n");
}


static void Test_ZEVars_Version1()
{
	#if 0
	/*
	> required:
		> a buffer to hold the variables themselves.
		> a buffer to hold the names of said variables.
	*/
	ZEBuffer buf = Buf_FromMalloc(malloc(MegaBytes(1)), MegaBytes(1));
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

static void* Test_Realloc(void* originalPtr, size_t numBytes)
{
	void* ptr = realloc(originalPtr, numBytes);
	printf(">>>>>>>>>>>> Realloced %d to %d - total %d\n",
		(u32)originalPtr, (u32)ptr, g_allocCount);
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
	ZEAllocator alloc = { Test_ZEVarMalloc, Test_Realloc, Test_ZEVarFree };
	char* path = "stats.txt";
	//char* path = "stats copy.txt";
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
	f = NULL;

	const i32 maxSets = 32;
	ZEVarSet* sets[maxSets];
	i32 numSets = 0;

	ZEVar_ReadFromText((char*)mem, strLen, alloc, sets, &numSets, maxSets);
	printf("---------------------------\n");
	printf("Create %d sets\n", numSets);
	for (i32 i = 0; i < numSets; ++i)
	{
		ZEVar_List(sets[i]);
		//ZStr_PrintChars(sets[i]->data.start, sets[i]->data.Written(), 16);
	}
	alloc.Free(mem);

	// Test save
	char* saveFilePath = "stats_save_test.txt";
	err = fopen_s(&f, saveFilePath, "w");
	for (i32 i = 0; i < numSets; ++i)
	{
		ZEVar_WriteToTextFile(f, sets[i]);
	}
	i32 bytesWritten = ftell(f);
	printf("Wrote %d sets (%d bytes) to %s\n", numSets, bytesWritten, saveFilePath);
	fclose(f);
}



static void Test_ZEVars_Version2_b()
{
	printf("--- ZE Set Version 2 ---\n");

	ZEVarSet* readTest = NULL;
	ZEAllocator alloc = { Test_ZEVarMalloc, Test_Realloc, Test_ZEVarFree };

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

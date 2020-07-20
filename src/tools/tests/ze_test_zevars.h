#include "../../ze_common/ze_vars.h"

static void ZEVar_List(ZEByteBuffer* b)
{
	printf("--- List ZEVars ---\n");
	u8* read = b->start;
	u8* end = b->cursor;
	while (read < end)
	{
		ZEVar* v = (ZEVar*)read;
		if (v->sentinel != ZE_SENTINEL)
		{ printf("\tdesync - read vars failed\n"); return; }
		read += v->size;
		i32 offset = (i32)((u8*)v - b->start);
		switch(v->type)
		{
			case ZEVAR_TYPE_INT:
			printf("Int %d. Name %s, Offset %d\n",
				v->data.i, v->name, offset);
			break;
			case ZEVAR_TYPE_STR:
			printf("Str \"%s\". Len (%d) Name %s, Offset %d\n",
				v->data.txt.chars, v->data.txt.len, v->name, offset);
			break;
			default:
			printf("Unknown var type %d\n", v->type);
			break;
		}
	}
}

static void Test_ZEVars_Version1()
{
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
}

static void Test_ZEVars_Version2()
{
	printf("--- ZE Set Version 2 ---\n");
	char* intAName = "mob_health";
	char* intBName = "mob_damage";
	ZEVarSet set = ZEVar_CreateSet("goblin", 16, 1024);
	set.AddInt(intAName, 100);
	set.AddInt(intBName, 5);
	printf("%s: %d\n", intAName, set.GetInt(intAName, 0));
	printf("%s: %d\n", intBName, set.GetInt(intBName, 0));
	printf("Set %s: %d of %d keys, %d of %d bytes\n",
		set.name,
		set.table->m_numKeys, set.table->m_maxKeys,
		set.data.Written(), set.data.capacity);
}

static void Test_ZEVars()
{
	printf("\n=== Test ZEVars ===\n");
	Test_ZEVars_Version1();
	Test_ZEVars_Version2();
}

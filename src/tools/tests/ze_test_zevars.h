#include "../../ze_common/ze_vars.h"

static void ZEVar_List(ZEByteBuffer* b)
{
	printf("--- List ZEVars ---\n");
	u8* read = b->start;
	// name is stored at the front of the array so
	// advance until passed it
	read = (u8*)ZE_ReadToNullChar((char*)read);
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
			printf("Int %d. Name %s, Offset %d\n",
				v->data.i, v->name, offset);
			break;
			case ZEVAR_TYPE_FLOAT:
			printf("float %f. Name %s, Offset %d\n",
				v->data.f, v->name, offset);
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
	printf("\t%d vars, avg size %.3f\n", numVars, (f32)totalSize / (f32)numVars);
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

// example fields for mob definition
#define ZV_MOB_CLASS "mob_class"
#define ZV_MOB_HEALTH "mob_health"
#define ZV_MOB_DAMAGE "mob_damage"
#define ZV_MOB_SPEED "mob_speed"
#define ZV_MOB_STUN_HEALTH "mob_stun_health"
#define ZV_MOB_STUNTIME "mob_stun_time"
#define ZV_MOB_MOVE_TYPE "mob_move_type"

static void Test_ZEVars_Version2()
{
	printf("--- ZE Set Version 2 ---\n");
	ZEVarSet set = ZEVar_CreateSet("goblin", 16, 1024);
	set.AddInt(ZV_MOB_HEALTH, 100);
	set.AddInt(ZV_MOB_DAMAGE, 5);
	set.AddString(ZV_MOB_CLASS, "class_goblin");
	set.AddFloat(ZV_MOB_SPEED, 5.5f);
	set.AddInt(ZV_MOB_STUN_HEALTH, 1);
	set.AddFloat(ZV_MOB_STUNTIME, 2.f);
	set.AddString(ZV_MOB_MOVE_TYPE, "walk");

	ZEVar_List(&set.data);

	printf("%s: %d\n", ZV_MOB_HEALTH, set.GetInt(ZV_MOB_HEALTH, 0));
	printf("%s: %d\n", ZV_MOB_DAMAGE, set.GetInt(ZV_MOB_DAMAGE, 0));
	printf("%s: %s\n", ZV_MOB_CLASS, set.GetString(ZV_MOB_CLASS));
	printf("%s: %.3f\n", ZV_MOB_SPEED, set.GetFloat(ZV_MOB_SPEED, 0));
	printf("Set %s: %d of %d keys, %d of %d bytes\n",
		set.name,
		set.table->m_numKeys, set.table->m_maxKeys,
		set.data.Written(), set.data.capacity);
}

static void Test_ZEVars()
{
	printf("\n=== Test ZEVars ===\n");
	//Test_ZEVars_Version1();
	Test_ZEVars_Version2();
}

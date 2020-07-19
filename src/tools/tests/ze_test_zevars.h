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

static void Test_ZEVars()
{
	printf("\n=== Test ZEVars ===\n");

	/*
	> required:
		> a buffer to hold the variables themselves.
		> a buffer to hold the names of said variables.
	*/
	ZEByteBuffer buf = Buf_FromMalloc(malloc(MegaBytes(1)), MegaBytes(1));
	i32 c = ZEVar_AddString(&buf, "mob_class", "base_goblin");
	i32 a = ZEVar_AddInt(&buf, "mob_health", 100);
	i32 b = ZEVar_AddInt(&buf, "mob_run_speed", 5);
	ZEVar_List(&buf);
}

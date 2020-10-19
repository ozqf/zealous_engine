#pragma once

#include "../../ze_common/ze_common_full.h"

///////////////////////////////////////////////////////
// Introspection data
///////////////////////////////////////////////////////

#define INTROSPECTION_TYPE_NULL 0
#define INTROSPECTION_TYPE_BYTES 1
#define INTROSPECTION_TYPE_INT32 2
#define INTROSPECTION_TYPE_FLOAT32 3
#define INTROSPECTION_TYPE_VECTOR3 4
#define INTROSPECTION_TYPE_NORMAL3 5

struct EncodeVar
{
	// type describes how to read/write this value
	i32 type;
	// size in bytes of the data field in the struct
	i32 size;
	// offset from start of structure to start of this value
	i32 offset;
	
	// for debugging
	char* label;
};

struct EncodeTable
{
	// size of all members tallied up
	i32 maxSize;
	char* label;
	// 32 max for bitmasking
	EncodeVar vars[32];
	i32 numVars;
};

internal i32 Test_GetIntrospectionTableSize(EncodeTable* t)
{
	i32 total = 0;
	for (i32 i = 0; i < t->numVars; ++i)
	{
		total += t->vars[i].size;
	}
	return total;
}

internal void Test_SetTableVar(
	EncodeTable* table, i32 index, i32 type, i32 size, i32 offset, char* label)
{
	// validate
	if (size <= 0)
	{
		printf("FATAL: Zero size variable: %s\n", label);
		ILLEGAL_CODE_PATH
	}
	if (type <= 0)
	{
		printf("FATAL: Unknown type %d specified for var %s\n",
			type, label);
		ILLEGAL_CODE_PATH
	}

	// overwrite var count if necessary
	i32 newMaxVar = index + 1;
	if (table->numVars < newMaxVar)
	{
		table->numVars = newMaxVar;
	}
	EncodeVar* v = &table->vars[index];
	v->offset = offset;
	v->type = type;
	v->size = size;
	v->label = label;
}

internal i32 Test_CalcDiff(EncodeTable* t,  u8* a, u8* b)
{
	i32 bits = 0;
	for (i32 i = 0; i < t->numVars; ++i)
	{
		EncodeVar* v = &t->vars[i];
		u8* ptrA = (a + v->offset);
		u8* ptrB = (b + v->offset);
		//printf("Compare field %s\n", v->label);
		//COM_PrintBytes(ptrA, v->size, 16);
		//COM_PrintBytes(ptrB, v->size, 16);

		if (!ZE_CompareMemory(ptrA, ptrB, v->size))
		{
			bits |= (1 << i);
		}
	}
	return bits;
}

// returns bytes written
internal i32 Test_WriteTestTypeDelta(
	EncodeTable* table, u8* itemBytes, i32 diffBits, u8* buffer)
{
	printf("Encoding...\n");
	u8* cursor = buffer;
	cursor += COM_WriteI32(diffBits, cursor);
	for (i32 i = 0; i < table->numVars; ++i)
	{
		i32 bit = (1 << i);
		if ((diffBits & bit) != 0)
		{
			EncodeVar* var = &table->vars[i];
			switch (var->type)
			{
				case INTROSPECTION_TYPE_INT32:
				{
					i32 value = *(i32*)(itemBytes + var->offset);
					printf("Write i32 field %d %s: %d\n",
						i, var->label, value);
					cursor += COM_WriteI32(value, cursor);
				} break;
				case INTROSPECTION_TYPE_BYTES:
				{
					printf("Write bytes field %d %s\n", i, var->label);
					cursor += ZE_Copy(
						itemBytes + var->offset,
						cursor,
						var->size);
				} break;
				default:
				{
					printf("Field \"%s\" type %d is unsupported\n",
						var->label, var->type);
				} break;
			}
		}
	}
	return (cursor - buffer);
}

// returns bytes read
internal i32 Test_ReadTestTypeDelta(EncodeTable* table, u8* itemBytes, u8* buffer)
{
	u8* cursor = buffer;
	i32 diffBits = COM_ReadI32(&cursor);
	for (i32 i = 0; i < table->numVars; ++i)
	{
		i32 bit = diffBits & (1 << i);
		if (bit)
		{
			EncodeVar* var = &table->vars[i];
			switch (var->type)
			{
				case INTROSPECTION_TYPE_INT32:
				{
					i32 value = COM_ReadI32(&cursor);
					printf("Read i32 field %d %s: %d\n", i, var->label, value);
					*(i32*)(itemBytes + var->offset) = value;
				} break;
				case INTROSPECTION_TYPE_BYTES:
				{
					printf("Read bytes field %d %s\n", i, var->label);
					cursor += ZE_Copy(cursor, itemBytes + var->offset, var->size);
				} break;
				default:
				{
					printf("Field \"%s\" type %d is unsupported\n", var->label, var->type);
				} break;
			}
		}
	}
	return (cursor - buffer);
}

/*internal*/ EncodeTable* Test_GetEncodeTable(
	EncodeTable* tables, i32 numTables, char* name)
{
	for (i32 i = 0; i < numTables; ++i)
	{
		if (ZStr_Compare(tables[i].label, name))
		{
			return &tables[i];
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////
// Test structs
///////////////////////////////////////////////////////

struct TestType
{
	i32 a;
	i32 b;
	i32 c;

	f32 x;
	f32 y;
	f32 z;
	
	i32 state;
};

internal EncodeTable g_encodeTables[13];
internal EncodeTable TestType_Table;

#define ADD_FIELD(ptrTable, varIndex, ptrStruct, varType, varName, sizeInBytes) \
{ \
	char* varLabel = #varName##; \
	i32 offsetInStruct = (i32)((u8*)&##ptrStruct##->##varName - (u8*)##ptrStruct##); \
	Test_SetTableVar( \
		ptrTable##, varIndex##, varType##, sizeInBytes##, offsetInStruct, varLabel \
	); \
}


internal void Test_BuildTestTypeIntrospectionTable(EncodeTable* t)
{
	TestType item = {};
	TestType* ptrItem = &item;
	
	u8* structPtr = (u8*)ptrItem;
	u8* structMemberPtr = (u8*)&ptrItem->a;
	printf("Struct ptr %d member ptr: %d\n", (i32)structPtr, (i32)structMemberPtr);

	// Via macro
	ADD_FIELD(t, 0, (&item), INTROSPECTION_TYPE_BYTES, a, sizeof(i32))
	ADD_FIELD(t, 1, (&item), INTROSPECTION_TYPE_BYTES, b, sizeof(i32))
	ADD_FIELD(t, 2, (&item), INTROSPECTION_TYPE_BYTES, c, sizeof(i32))
	ADD_FIELD(t, 3, (&item), INTROSPECTION_TYPE_BYTES, x, sizeof(i32))
	ADD_FIELD(t, 4, (&item), INTROSPECTION_TYPE_BYTES, y, sizeof(i32))
	ADD_FIELD(t, 5, (&item), INTROSPECTION_TYPE_BYTES, z, sizeof(i32))
	ADD_FIELD(t, 6, (&item), INTROSPECTION_TYPE_BYTES, state, sizeof(i32))
	t->maxSize = Test_GetIntrospectionTableSize(t);
}

///////////////////////////////////////////////////////
// Run Tests
///////////////////////////////////////////////////////

internal void Test_PrintIntrospectionTable(EncodeTable* table)
{
	printf("--- %s table contents (%d bytes)---\n",
		table->label, Test_GetIntrospectionTableSize(table));
	for (i32 i = 0; i < table->numVars; ++i)
	{
		EncodeVar* var = &table->vars[i];
		printf("%s: type %d size %d offset %d\n",
			var->label, var->type, var->size, var->offset);
	}
	printf("\n");
}

internal void Test_PrintTestType(TestType* t)
{
	printf("a %d, b %d, c %d, x %f, y %f, z %f state: %d\n",
		t->a, t->b, t->c, t->x, t->y, t->z, t->state
	);
}

void COM_PrintBits(i32 val, u8 printNewLine)
{
    for (i32 i = 31; i >= 0; --i)
    {
        i32 result = 1 & (val >> i);
        printf("%d", result);
    }
    if (printNewLine)
    {
        printf("\n");
    }
}

void COM_PrintBytes(u8* bytes, i32 numBytes, i32 bytesPerRow)
{
    u8* read = bytes;
    if (bytesPerRow <= 0)
    {
        bytesPerRow = 16;
    }
    u8* end = read + numBytes;
    i32 count = 0;
    while (read < end)
    {
        printf("%03d, ", *read);
        read++;
        if (++count >= bytesPerRow)
        {
            count = 0;
        }
    }
    printf("\n");
}


internal void Test_Introspection()
{
	printf("\n === Test delta introspection ===\n\n");
	TestType previousState = {};

	printf("Number of encoding tables: %d\n", (sizeof(g_encodeTables) / sizeof(*g_encodeTables)));

	TestType_Table.label = "TestType";
	EncodeVar* vars = TestType_Table.vars;

	EncodeTable* t = &TestType_Table;
	Test_BuildTestTypeIntrospectionTable(t);
	Test_PrintIntrospectionTable(t);
	
	previousState.a = 567;
	previousState.b = 81;
	previousState.c = -14;
	previousState.y = 5903.359f;

	TestType currentState = {};
	currentState.a = 567;
	currentState.b = -71;
	currentState.c = -14;
	currentState.y = 2458.038f;
	currentState.state = 43;
	
	printf("Items:\n");
	Test_PrintTestType(&previousState);
	Test_PrintTestType(&currentState);
	#if 1
	// Force a full state write by sending a full diff mask:
	//i32 diffBits = 0xffffffff;
	i32 diffBits = Test_CalcDiff(t, (u8*)&previousState, (u8*)&currentState);
	printf("Diff bits (%d):\n", diffBits);
	COM_PrintBits(diffBits, 1);
	u8 buffer[512];
	u8* cursor = buffer;
	
	cursor += Test_WriteTestTypeDelta(t, (u8*)&currentState, diffBits, cursor);
	i32 written = (cursor - buffer);
	printf("Wrote %d bytes\n", written);
	COM_PrintBytes(buffer, (u16)written, 4);

	printf("Reconstruct from blank:\n");
	TestType result = {};
	cursor = buffer;
	cursor += Test_ReadTestTypeDelta(t, (u8*)&result, cursor);
	Test_PrintTestType(&result);

	#endif
}

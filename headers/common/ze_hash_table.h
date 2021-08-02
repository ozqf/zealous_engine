#ifndef ZE_BLOB_HASH_TABLE_H
#define ZE_BLOB_HASH_TABLE_H

#include "../ze_common.h"

#define ZE_LT_SCALE 2

#define ZE_LT_INVALID_INDEX -1
#define ZE_LT_INVALID_ID 0

#ifndef ZE_LT_SET_ZERO
#define ZE_LT_SET_ZERO(ptrToSet, numBytesToZero) \
    memset(##ptrToSet##, 0, numBytesToZero##)
#endif

// uncomment this for TESTING ONLY
//#define ZE_LT_USE_BAD_HASH_FUNCTION

#ifndef ZE_LT_USE_BAD_HASH_FUNCTION
// Credit: Thomas Wang
// https://burtleburtle.net/bob/hash/integer.html
static u32 ZE_LT_HashUint(u32 a)
{
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    return a;
}
#endif

// DO NOT USE!
// This is a deliberately broken hash for testing
// hash collision resolution!
#ifdef ZE_LT_USE_BAD_HASH_FUNCTION
static u32 ZE_LT_HashUint(u32 a)
{
    return 1;
}
#endif

union ZEHashTableData
{
    f32 f;
    i32 i;
    u32 u;
    f64 bigF;
    i64 bigI;
    u64 bigU;
    void* ptr;
};

struct ZEHashTableKey
{
    i32 id;
    u32 idHash;
    i32 collisionsOnInsert;
    ZEHashTableData data;
};

struct ZEHashTable
{
    ZEHashTableKey *m_keys;
    i32 m_numKeys;
    i32 m_maxKeys;

    //////////////////////////////////////////////////
    // insert/find/remove keys
    //////////////////////////////////////////////////
    void StepKeyIndex(i32 *index)
    {
        *index += 1;
        if (*index >= m_maxKeys)
        {
            *index = 0;
        }
    }

    void ClearKey(ZEHashTableKey *key)
    {
        *key = {};
        key->id = ZE_LT_INVALID_ID;
    }

    void Clear()
    {
        for (i32 i = 0; i < m_maxKeys; ++i)
        {
            ClearKey(&m_keys[i]);
        }
    }

    i32 FindKeyIndex(i32 id)
    {
        u32 hash = ZE_LT_HashUint(id);
        i32 keyIndex = hash % m_maxKeys;
        i32 escape = 0;
        do
        {
            ZEHashTableKey *key = &m_keys[keyIndex];
            if (key->id == id)
            {
                return keyIndex;
            }
            else if (key->id == ZE_LT_INVALID_ID)
            {
                return ZE_LT_INVALID_INDEX;
            }
            else
            {
                StepKeyIndex(&keyIndex);
            }

        } while (escape++ < m_maxKeys);
        return ZE_LT_INVALID_INDEX;
    }

    i32 Insert(i32 id, ZEHashTableData data)
    {
        u32 idHash = ZE_LT_HashUint(id);
        i32 keyIndex = idHash % m_maxKeys;
        i32 escape = 0;
        u32 numCollisions = 0;
        do
        {
            ZEHashTableKey *key = &m_keys[keyIndex];
            if (key->id == ZE_LT_INVALID_ID)
            {
                // insert
                key->id = id;
                key->idHash = idHash;
                key->data = data;
                key->collisionsOnInsert = numCollisions;
                m_numKeys++;
                return ZE_ERROR_NONE;
            }
            else if (key->id == id)
            {
                // already inserted. Update data
                key->data = data;
                return ZE_ERROR_NONE;
            }
            else
            {
                // probe forward
                numCollisions++;
                StepKeyIndex(&keyIndex);
            }

        } while (escape++ < m_maxKeys);
        return ZE_ERROR_FUNC_RAN_AWAY;
    }

    i32 Remove(i32 id)
    {

        i32 keyIndex = FindKeyIndex(id);
        if (keyIndex == ZE_LT_INVALID_INDEX)
        {
            return ZE_ERROR_NOT_FOUND;
        }
        ClearKey(&m_keys[keyIndex]);
        m_numKeys--;

        // probe forward reinserting until a key
        // with an invalid id is found
        StepKeyIndex(&keyIndex);
        i32 escape = 0;
        do
        {
            ZEHashTableKey *key = &m_keys[keyIndex];
            if (key->id == ZE_LT_INVALID_ID)
            {
                // empty key. Done
                return ZE_ERROR_NONE;
            }
            i32 correctIndex = key->idHash % m_maxKeys;
            if (correctIndex != keyIndex)
            {
                // reinsert key
                ZEHashTableKey copy = *key;
                ClearKey(key);
                m_numKeys--; // decrement count as insert will increment it again
                Insert(copy.id, copy.data);
            }
            StepKeyIndex(&keyIndex);
        } while (escape++ < m_maxKeys);

        return ZE_ERROR_FUNC_RAN_AWAY;
    }

    //////////////////////////////////////////////////
    // Get/Set utility functions
    //////////////////////////////////////////////////
    ZEHashTableData *FindData(i32 id)
    {
        i32 keyIndex = FindKeyIndex(id);
        if (keyIndex == ZE_LT_INVALID_INDEX)
        {
            return NULL;
        }
        return &m_keys[keyIndex].data;
    }

    void *FindPointer(i32 id)
    {
        ZEHashTableData *data = FindData(id);
        if (data == NULL)
        {
            return NULL;
        }
        return data->ptr;
    }

    i32 FindI32(i32 id, i32 invalidResult)
    {
        ZEHashTableData *data = FindData(id);
        if (data == NULL)
        {
            return invalidResult;
        }
        return data->i;
    }

    i32 SetData(i32 id, ZEHashTableData data)
    {
        i32 keyIndex = FindKeyIndex(id);
        if (keyIndex == ZE_LT_INVALID_INDEX)
        {
            return ZE_ERROR_NOT_FOUND;
        }
        m_keys[keyIndex].data = data;
        return ZE_ERROR_NONE;
    }

    i32 SetI32(i32 id, i32 value)
    {
        ZEHashTableData* d = FindData(id);
        if (d == NULL) { return ZE_ERROR_NOT_FOUND; }
        d->i = value;
        return ZE_ERROR_NONE;
    }
};

static i32 ZE_HashTable_CalcBytesForTable(i32 numKeys)
{
    i32 bytesForKeys = sizeof(ZEHashTableKey) * numKeys;
    i32 bytesTotal = sizeof(ZEHashTable) + bytesForKeys;
    return bytesTotal;
}

/**
 * Initialise a table
 * > if mem is NULL, the table will be allocated
 *   > allocation is |table struct|...keys...|
 * > use CalcBytes function to measure required space!
 * > capacity passed in should be double size of the target
 * 		array to reduce collisions
 * 	eg 32 item array should have a lookup keys set of
 * 		 64 to avoid key collisions
 */
static ZEHashTable *ZE_HashTable_Create(
    ZE_mallocFunction mallocFn,
    i32 capacity, u8 *mem)
{
    if (mem == NULL)
    {
        i32 numBytes = ZE_HashTable_CalcBytesForTable(capacity);
        mem = (u8 *)mallocFn(numBytes);
    }
    ZEHashTable *table = (ZEHashTable *)mem;
    *table = {};
    table->m_maxKeys = capacity;
    table->m_keys = (ZEHashTableKey *)(mem + sizeof(ZEHashTable));
    table->Clear();
    return table;
}

static void ZE_HashTable_Delete(ZEHashTable *table, ZE_freeFunction freeFn)
{
    freeFn(table);
}

#endif // ZE_BLOB_HASH_TABLE_H
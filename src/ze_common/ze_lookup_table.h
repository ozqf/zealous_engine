#ifndef ZE_LOOKUP_TABLE_H
#define ZE_LOOKUP_TABLE_H

#include "ze_common.h"

#define ZE_LT_SCALE 2

#define ZE_LT_INVALID_INDEX -1
#define ZE_LT_INVALID_ID 0

#ifndef ZE_LT_MALLOC
#define ZE_LT_MALLOC(numBytesToAlloc) \
(u8*)malloc(##numBytesToAlloc##)
#endif

#ifndef ZE_LT_FREE
#define ZE_LT_FREE(ptrToFree) \
free(##ptrToFree##)
#endif

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

struct ZELookupKey
{
    i32 id;
    i32 data;
    u32 idHash;
    i32 collisionsOnInsert;
};

struct ZELookupTable
{
    ZELookupKey* m_keys;
    i32 m_numKeys;
    i32 m_maxKeys;
    i32 m_invalidDataValue;

    void StepKeyIndex(i32* index)
    {
        *index += 1;
        if (*index >= m_maxKeys)
        {
            *index = 0;
        }
    }

    void ClearKey(ZELookupKey* key)
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
            ZELookupKey* key = &m_keys[keyIndex];
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

    i32 FindData(i32 id)
    {
        i32 keyIndex = FindKeyIndex(id);
        if (keyIndex == ZE_LT_INVALID_INDEX) { return m_invalidDataValue; }
        return m_keys[keyIndex].data;
    }

    i32 SetData(i32 id, i32 data)
    {
        i32 keyIndex = FindKeyIndex(id);
        if (keyIndex == ZE_LT_INVALID_INDEX) { return COM_ERROR_NOT_FOUND; }
        m_keys[keyIndex].data = data;
        return COM_ERROR_NONE;
    }

    i32 Insert(i32 id, i32 data)
    {
        u32 idHash = ZE_LT_HashUint(id);
        i32 keyIndex = idHash % m_maxKeys;
        i32 escape = 0;
        u32 numCollisions = 0;
        do
        {
            ZELookupKey* key = &m_keys[keyIndex];
            if (key->id == ZE_LT_INVALID_ID)
            {
                // insert
                key->id = id;
                key->idHash = idHash;
                key->data = data;
                key->collisionsOnInsert = numCollisions;
                m_numKeys++;
                return COM_ERROR_NONE;
            }
            else if (key->id == id)
            {
                // already inserted. Update data
                key->data = data;
                return COM_ERROR_NONE;
            }
            else
            {
                // probe forward
                numCollisions++;
                StepKeyIndex(&keyIndex);
            }
            
        } while (escape++ < m_maxKeys);
        return COM_ERROR_FUNC_RAN_AWAY;
    }

    i32 Remove(i32 id)
    {
        
        i32 keyIndex = FindKeyIndex(id);
        if (keyIndex == ZE_LT_INVALID_INDEX) { return COM_ERROR_NOT_FOUND; }
        ClearKey(&m_keys[keyIndex]);
        m_numKeys--;

        // probe forward reinserting until a key
        // with an invalid id is found
        StepKeyIndex(&keyIndex);
        i32 escape = 0;
        do
        {
            ZELookupKey* key = &m_keys[keyIndex];
            if (key->id == ZE_LT_INVALID_ID)
            {
                // empty key. Done
                return COM_ERROR_NONE;
            }
            i32 correctIndex = key->idHash % m_maxKeys;
            if (correctIndex != keyIndex)
            {
                // reinsert key
                ZELookupKey copy = *key;
                ClearKey(key);
                m_numKeys--; // decrement count as insert will increment it again
                Insert(copy.id, copy.data);
            }
            StepKeyIndex(&keyIndex);
        } while (escape++ < m_maxKeys);
        
        return COM_ERROR_FUNC_RAN_AWAY;
    }
};

static i32 ZE_LT_CalcBytesForTable(i32 numKeys)
{
    i32 bytesForKeys = sizeof(ZELookupKey) * numKeys;
    i32 bytesTotal = sizeof(ZELookupTable) + bytesForKeys;
    return bytesTotal;
}

/**
 * Initialise a table
 * > if mem is NULL, the table will be allocated
 *   > allocation is |table struct|...keys...|
 * > use CalcBytes function to measure required space!
 * > capacity passed in should be double size of the lookup array to reduce collisions
 */
static ZELookupTable* ZE_LT_Create(i32 capacity, i32 invalidDataValue, u8* mem)
{
    if (mem == NULL)
    {
        i32 numBytes = ZE_LT_CalcBytesForTable(capacity);
        mem = (u8*)ZE_LT_MALLOC(numBytes);
    }
    ZELookupTable* table = (ZELookupTable*)mem;
    *table = {};
    table->m_maxKeys = capacity;
    table->m_invalidDataValue = invalidDataValue;
    table->m_keys = (ZELookupKey*)(mem + sizeof(ZELookupTable));
    table->Clear();
    return table;
}

#endif // ZE_LOOKUP_TABLE_H
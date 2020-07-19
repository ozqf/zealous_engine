#ifndef ZE_LOOKUP_STRING_TABLE_H
#define ZE_LOOKUP_STRING_TABLE_H

#include "ze_common.h"
#include "ze_string_utils.h"
#include "ze_byte_buffer.h"
#include "ze_hash.h"

#define ZE_MAX_KEY_LENGTH 32

struct ZELookupStrKey
{
    u32 keyHash;
	char key[ZE_MAX_KEY_LENGTH];
	i32 data;
};

struct ZELookupStrTable
{
    ZELookupStrKey* m_keys;
    i32 m_numKeys;
    i32 m_maxKeys;
	ZEByteBuffer data;

    void StepKeyIndex(i32* index)
    {
        *index += 1;
        if (*index >= m_maxKeys)
        {
            *index = 0;
        }
    }

	i32 FindKeyIndex(char* querykey)
	{
		u32 queryHash = ZE_Hash_djb2((u8*)querykey);
		i32 queryIndex = queryHash % m_maxKeys;
		i32 escape = 0;
		do
		{
			ZELookupStrKey* key = &m_keys[queryIndex];
			// Found it, woo
			if (key->keyHash == queryHash)
			{
				return queryIndex;
			}
			else if (key->keyHash == 0)
			{
				// should be here but isn't...
				return -1;
			}
			else
			{
				StepKeyIndex(&queryIndex);
			}
			
		} while (escape++ < m_maxKeys);
		return -1;
	}

	i32 GetData(char* key, i32 fail)
	{
		i32 index = FindKeyIndex(key);
		if (index == -1) { return fail; }
		return m_keys[index].data;
	}

	ErrorCode Insert(char* newKey, i32 newData)
	{
		//i32 len = ZE_StrLen(newKey);
		u32 newKeyHash = ZE_Hash_djb2((u8*)newKey);
		i32 keyIndex = newKeyHash % m_maxKeys;
		i32 escape = 0;
		do
		{
			ZELookupStrKey* key = &m_keys[keyIndex];
			// empty
			if (key->keyHash == 0)
			{
				// insert
				ZE_CopyStringLimited(newKey, key->key, ZE_MAX_KEY_LENGTH);
				key->keyHash = newKeyHash;
				key->data = newData;
				m_numKeys++;
				return ZE_ERROR_NONE;
			}
			// update
			else if (key->keyHash == newKeyHash)
			{
				key->data = newData;
				return ZE_ERROR_NONE;
			}
			// probe forward
			else
			{
				StepKeyIndex(&keyIndex);
			}
			
		} while (escape++ < m_maxKeys);
		return ZE_ERROR_FUNC_RAN_AWAY;
	}
};

/**
 * Returns bytes to store
 */
static i32 ZE_MeasureStringHashTable(i32 capacity)
{
	i32 maxKeys = capacity * 2;
	return sizeof(ZELookupStrTable) + (sizeof(ZELookupStrKey) * maxKeys);  
}

static ZELookupStrTable* ZE_CreateStringHashTable(i32 capacity, u8* ptr)
{
	i32 maxKeys = capacity * 2;
	i32 total = sizeof(ZELookupStrTable) + (sizeof(ZELookupStrKey) * maxKeys);
	if (ptr == NULL)
	{
		ptr = (u8*)malloc(total);
	}
	// clear memory to remove garbage hashes etc
	ZE_SET_ZERO(ptr, total)
	ZELookupStrTable* table = (ZELookupStrTable*)ptr;
	table->m_keys = (ZELookupStrKey*)(ptr + sizeof(ZELookupStrTable));
	table->m_maxKeys = maxKeys;
	table->m_numKeys = 0;
	return table;
}

#endif // ZE_LOOKUP_STRING_TABLE_H

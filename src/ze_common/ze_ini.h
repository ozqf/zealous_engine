#ifndef ZE_INI_H
#define ZE_INI_H

#include "ze_common.h"
#include "ze_lookup_table.h"
#include "ze_byte_buffer.h"
#include "ze_hash.h"
#include "ze_vars.h"

// struct ZEIniFile
// {
// 	ZELookupTable* table;
// 	ZEBuffer* data;
// };

static char* ZEIni_GetValue(
	ZELookupTable* table, ZEBuffer* data,
	const char* section, const char* field)
{
	i32 hash = ZE_Hash_djb2_pair((u8*)section, (u8*)field);
	i32 offset = table->FindData(hash);
	if (offset == table->m_invalidDataValue)
	{ return NULL; }
	ZEIntern* intern = (ZEIntern*)data->GetAtOffset(offset);
	return (char*)data->GetAtOffset(intern->charsOffset);
}

#endif // ZE_INI_H
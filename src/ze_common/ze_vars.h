#ifndef ZE_VARS_H
#define ZE_VARS_H

#include "ze_common.h"
#include "ze_byte_buffer.h"

//////////////////////////////////////////
// Data types
//////////////////////////////////////////

struct ZEVar
{
	i32 type;
	i32 nameHash;
	char* name;
};

struct ZEVarInt
{
	ZEVar header;
	i32 data;
};

struct ZEVarString
{
	ZEVar header;
	char* str;
	i32 numChars;
};

struct ZEVarSet
{
	i32 numItems;
	char* name;
};

#endif // ZE_VARS_H

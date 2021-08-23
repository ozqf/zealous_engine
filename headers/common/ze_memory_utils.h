#ifndef ZE_MEMORY_UTILS_H
#define ZE_MEMORY_UTILS_H

#include "ze_common.h"

// set this to internal rather than inline to
// kill your FPS in serialisation functions
#define ZE_MEM_FUNC_INTERNAL inline

ZE_MEM_FUNC_INTERNAL u32 SafeTruncateUInt64(u64 value)
{
	ZE_ASSERT(value <= 0xFFFFFFFF, "Truncate U64 overflow");
	u32 result = (u32)value;
	return result;
}

////////////////////////////////////////////////////////////
// NOTE: None of this handles eddianness
////////////////////////////////////////////////////////////

/**
 * Returns number of bytes written
 */
ZE_MEM_FUNC_INTERNAL u32 ZE_WriteByte(i8 value, i8* target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    *target = value;
    return sizeof(i8);
}

/**
 * Returns number of bytes written
 */
ZE_MEM_FUNC_INTERNAL void ZE_WriteByte(i8 value, i8** target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    **target = value;
    *target++;
}

/**
 * Returns number of bytes written
 */
ZE_MEM_FUNC_INTERNAL u32 ZE_WriteI16(i16 value, i8* target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    *(i16*)target = value;
    return sizeof(u16);
}

/**
 * Returns number of bytes written
 */
ZE_MEM_FUNC_INTERNAL u32 ZE_WriteU16(u16 value, i8* target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    *(u16*)target = value;
    return sizeof(u16);
}

/**
 * Returns number of bytes written
 */
ZE_MEM_FUNC_INTERNAL u32 ZE_WriteI32(i32 value, i8* target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    *(i32*)target = value;
    return sizeof(i32);
}

/**
 * Returns number of bytes written
 */
ZE_MEM_FUNC_INTERNAL u32 ZE_WriteU32(u32 value, i8* target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    *(u32*)target = value;
    return sizeof(u32);
}

/**
 * Returns number of bytes written
 */
ZE_MEM_FUNC_INTERNAL u32 ZE_WriteF32(f32 value, i8* target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    *(f32*)target = value;
    return sizeof(f32);
}

/**
 * Read an i32 at the target pointer position AND move the target pointer forward
 */
ZE_MEM_FUNC_INTERNAL i8 ZE_ReadByte(i8** target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    i8 result = *(i8*)*target;
    *target += sizeof(i8);
    return result;
}

ZE_MEM_FUNC_INTERNAL u16 ZE_ReadU16(i8** target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    u16 result = *(u16*)*target;
    *target += sizeof(u16);
    return result;
}

/**
 * Read an i32 at the target pointer position AND move the target pointer forward
 */
ZE_MEM_FUNC_INTERNAL i32 ZE_ReadI32(i8** target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    i32 result = *(i32*)*target;
    *target += sizeof(i32);
    return result;
}

ZE_MEM_FUNC_INTERNAL u32 ZE_ReadU32(i8** target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    u32 result = *(u32*)*target;
    *target += sizeof(u32);
    return result;
}

ZE_MEM_FUNC_INTERNAL f32 ZE_ReadF32(i8** target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    f32 result = *(f32*)*target;
    *target += sizeof(f32);
    return result;
}

/**
 * Read an i32 at the target position, keeping target in place
 */
ZE_MEM_FUNC_INTERNAL i32 ZE_PeekI32(i8* target)
{
	//ZE_ASSERT(target != NULL, "Target is null");
    i32 result = *(i32*)target;
    return sizeof(i32);
}

// returns 1 if two blocks of memory are identical. 0 if otherwise.
ZE_MEM_FUNC_INTERNAL i32 ZE_CompareMemory(i8* ptrA, i8* ptrB, u32 numBytes)
{
	i8* end = ptrA + numBytes;
	do
	{
		if (*ptrA != *ptrB)
		{
			return 0;
		}
		++ptrA;
		++ptrB;
	} while (ptrA < end);
	return 1;
}

#endif // ZE_MEMORY_UTILS_H
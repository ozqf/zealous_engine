#ifndef ZE_BITPACK_H
#define ZE_BITPACK_H

#include "ze_common.h"
#include "ze_math_types.h"
#include <math.h>

// 2 to the power of N bits - 1:
#define SIX_BIT_MASK 63
#define TEN_BIT_MASK 1023
#define SIXTEEN_BIT_MASK 65535
#define TWENTY_FOUR_BIT_MASK 16777215

struct QuantiseDef
{
    i32 halfMajorScale;
    i32 positiveMaximum;
    i32 majorBits;
    i32 minorBits;
    f32 minorScale;
};

struct QuantiseSet
{
    QuantiseDef pos;
    QuantiseDef vel;
    QuantiseDef rot;
};

inline u32 COM_QuantiseF2I(f32 input, QuantiseDef* def)
{
    // Remove sign
    input += def->halfMajorScale;
    // Clamp
    if (input > def->positiveMaximum) { input = (f32)def->positiveMaximum; }
    if (input < 0) { input = 0; }
    // Multiple by minor scale to preserve remaining space as decimal
    return (u32)(input * def->minorScale);
}

inline f32 COM_DequantiseI2F(u32 input, QuantiseDef* def)
{
    f32 output = (f32)input;
    output /= def->minorScale;
    output -=def->halfMajorScale;
    return output;
}

inline f32 COM_QuantiseF(f32 input, QuantiseDef* def)
{
    return COM_DequantiseI2F(COM_QuantiseF2I(input, def), def);
}

inline void COM_QuantiseVec3(Vec3* v, QuantiseDef* def)
{
    v->x = COM_QuantiseF(v->x, def);
    v->y = COM_QuantiseF(v->y, def);
    v->z = COM_QuantiseF(v->z, def);
}

inline void COM_QuantiseInit(
    QuantiseDef* def, const i32 halfRange, const u8 numBits)
{
    f32 fullRange = (f32)halfRange * 2;
    def->halfMajorScale = halfRange;
    def->positiveMaximum = (def->halfMajorScale * 2) - 1;
    def->majorBits = (i32)log2f(fullRange) + 1;
    def->minorBits = numBits - def->majorBits;
    //printf("Major bits %d minor bits %d\n", def->majorBits, def->minorBits);
    def->minorScale = powf(2, (f32)def->minorBits);
}

// Quantise the given float into an integer between
// -halfRange to +(halfRange - 1), within the given bits.
// spare bits are used for decimal precision
inline u32 COM_QuantiseF2I_Slow(
    f32 input, const i32 halfRange, const u8 numBits)
{
    QuantiseDef def = {};
    COM_QuantiseInit(&def, halfRange, numBits);
    return COM_QuantiseF2I(input, &def);
}

// Reverse the Quantisation
inline f32 COM_DequantiseI2F_Slow(
    i32 input, const i32 halfRange, const u8 numBits)
{
    QuantiseDef def = {};
    COM_QuantiseInit(&def, halfRange, numBits);
    return COM_DequantiseI2F(input, &def);
    #if 0
	// Calc precision
	f32 fullRange = (f32)(halfRange * 2);
	i32 majorBits = (i32)log2f(fullRange);
	i32 minorBits = numBits - majorBits;
	f32 scale = powf(2, (f32)minorBits);
	// unpack
	f32 output = (f32)input;
	output /= scale;
	output -= halfRange;
	return output;
    #endif
}


/**
 * Reference for bitwise operators because I keep forgetting
 * them
 */
u8 ZE_IsFlagDifferent(u32 flagsA, u32 flagsB, u32 flag)
{
    return ((flagsA & flag) != (flagsB & flag));
}

void ZE_DisableBits(u32* flags, u32 mask)
{
    // bitwise AND and bitwise NOT to disable bit
    // expanded: flags = flags & ~mask
    *flags &= ~mask;
}

void ZE_ToggleBit(u32* bits, u32 mask)
{
    *bits ^= mask;
}

// Used to generate the mask constants used above
i32 ZE_CreateBitmask(i32 numBits)
{
    i32 m = 0;
    i32 bit = 0;
    while(bit < numBits)
    {
        m |= (1 << bit);
        bit++;
    }
    return m;
}

/**
 * Pack a 3D normal vector into an int with 10 bit precision
 * + 1 to make the number positive
 * multiple by 500 to move each vector component into
 * the range 0 - 1000)
*/
i32 ZE_PackVec3NormalToI32(f32 x, f32 y, f32 z)
{
    i32 intX = (i32)((x + 1) * 500);
    i32 intY = (i32)((y + 1) * 500);
    i32 intZ = (i32)((z + 1) * 500);
    i32 r = 0;
    r |= (intX << 20);
    r |= (intY << 10);
    r |= (intZ << 0);
    return r;
}

i32 ZE_PackVec3NormalToI32(f32* vector3)
{
    return ZE_PackVec3NormalToI32(vector3[0], vector3[1], vector3[2]);
}

Vec3 ZE_UnpackVec3Normal(i32 source)
{
    Vec3 r = {};
    //i32 mask = COM_CreateBitmask(10);
    i32 mask = TEN_BIT_MASK;
    i32 intX = ((mask << 20) & source) >> 20;
    i32 intY = ((mask << 10) & source) >> 10;
    i32 intZ = ((mask << 0) & source) >> 0;
    r.x = (f32)intX / 500;
    r.y = (f32)intY / 500;
    r.z = (f32)intZ / 500;
    r.x -= 1;
    r.y -= 1;
    r.z -= 1;
    return r;
}


#endif // ZE_BITPACK_H
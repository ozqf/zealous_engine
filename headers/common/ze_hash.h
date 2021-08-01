#ifndef ZE_HASH_H
#define ZE_HASH_H

#include "../ze_common.h"

// source http://www.cse.yorku.ca/~oz/hash.html

static unsigned long ZE_Hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != '\0') // this line gives compiler warning
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

static unsigned long ZE_Hash_djb2_pair(unsigned char *str, unsigned char *strB)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != '\0') // this line gives compiler warning
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    while ((c = *strB++) != '\0') // this line gives compiler warning
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

static u32 ZE_Hash_djb2_Fixed(unsigned char *str, i32 numChars)
{
    u32 hash = 5381;
    int c;
    for (i32 i = 0; i < numChars; ++i)
    {
        c = *str++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

// Credit: Thomas Wang
// https://burtleburtle.net/bob/hash/integer.html
static u32 ZE_BurtleHashUint(u32 a)
{
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    return a;
}

#endif // ZE_HASH_H
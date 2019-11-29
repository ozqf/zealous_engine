#ifndef ZE_HASH_H
#define ZE_HASH_H

#include "ze_common.h"

// source http://www.cse.yorku.ca/~oz/hash.html
#if 0
unsigned long ZE_Hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++) // this line gives compiler warning
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
#endif
static unsigned long ZE_Hash_djb2_Fixed(unsigned char *str, i32 numChars)
{
    unsigned long hash = 5381;
    int c;
    for (i32 i = 0; i < numChars; ++i)
    {
        c = *str++;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

#endif // ZE_HASH_H
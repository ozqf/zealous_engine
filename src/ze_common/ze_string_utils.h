#ifndef ZE_STRING_UTILS_H
#define ZE_STRING_UTILS_H

#include "ze_common.h"

static i32 ZE_StrLenNoTerminator(const char* str)
{
    i32 count = 0;
    while (str[count]) { ++count; }
    return count;
}

/**
 * if strings are equal, return 0
 * if a is first alphabetically, return -1
 * if b is first alphabetically, return 1
 */

internal i32 ZE_CompareStrings(const char *a, const char *b)
{
    while (*a == *b)
    {
        // End of string
        if (*a == '\0')
        {
            return 0;
        }
        ++a;
        ++b;
    }

    return ((*a < *b) ? -1 : 1);
}

/**
 * Copy a string without exceeding the specified limit
 * Will always patch a NULL terminator at the final position
 */
internal i32 ZE_CopyStringLimited(const char *source, char *target, i32 limit)
{
    if (limit < 1) { return 0; }
    i32 written = 0;
    while (*source && limit > 0)
    {
        *target++ = *source++;
        --limit;
        ++written;
        //if (limit == 0) { break; }
    }
	target[limit - 1] = '\0';
    return ++written;
}

#endif // ZE_STRING_UTILS_H
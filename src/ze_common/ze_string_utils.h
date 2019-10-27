#ifndef ZE_STRING_UTILS_H
#define ZE_STRING_UTILS_H

#include "ze_common.h"

static i32 ZE_StrLenNoTerminator(const char* str)
{
    i32 count = 0;
    while (str[count]) { ++count; }
    return count;
}


#endif // ZE_STRING_UTILS_H
#ifndef ZE_STRING_UTILS_H
#define ZE_STRING_UTILS_H

#include "ze_common.h"

/////////////////////////////////////////////
// general parsing/formatting utils
/////////////////////////////////////////////

static i32 ZStr_LenNoTerminator(const char* str)
{
    i32 count = 0;
    while (str[count]) { ++count; }
    return count;
}

static i32 ZStr_Len(const char* str)
{
    i32 count = 0;
    while (str[count]) { ++count; }
    // include terminator
    return count + 1;
}

static Point2 ZStr_FindToken(const char* str, i32 tokenIndex, char separator)
{
    Point2 p = {};
    i32 currentTokenIndex = 0;
    i32 i = 0;
    i32 bInToken = NO;
    char c;
    while ((c = str[i++]) != '\0')
    {
        if (bInToken)
        {
            if (c == separator)
            {
                bInToken = NO;
                if (currentTokenIndex == tokenIndex)
                {
                    p.y = i - 1;
                    return p;
                }
                currentTokenIndex += 1;
            }
            continue;
        }
        else
        {
            if (c != separator)
            {
                bInToken = YES;
                if (currentTokenIndex == tokenIndex)
                {
                    p.x = i - 1;
                }
            }
        }
    }
    return p;
}

static char ZStr_CharToLower(char c)
{
    if (c >= 65 && c <= 90)
    {
        return c + 32;
    }
    return c;
}
#if 0
#include <ctype.h>
static void ZStr_ToLower(char* str)
{
    while (*str != '\0')
    {
        *str = (char)tolower(*str);
        str++;
    }
}
#endif

internal i32 ZStr_Equal(const char *a, const char *b)
{
    while (*a == *b)
    {
        if (*a == '\0')
        {
            return (*a == *b);
        }
        ++a;
        ++b;
    }
    return NO;
}

/**
 * if strings are equal, return 0
 * if a is first alphabetically, return -1
 * if b is first alphabetically, return 1
 */

internal i32 ZStr_Compare(const char *a, const char *b)
{
    while (*a == *b)
    {
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
 * if strings are equal, return 0
 * if a is first alphabetically, return -1
 * if b is first alphabetically, return 1
 */

internal i32 ZStr_CompareNocase(
    const char *a, const char *b)
{
    i32 i = 0;
    char aLower = 0;
    char bLower = 0;
    for(;;)
    {
        if (*a == '\0')
		{
			if (*b == '\0') { return 0; }
			else { return 1; }
		}
		else if (*b == '\0') { return -1; }
		
        aLower = ZStr_CharToLower(*a);
        bLower = ZStr_CharToLower(*b);
        if (aLower != bLower)
        {
            break;
        }
		a++;
		b++;
    }
    return ((aLower < bLower) ? -1 : 1);
}

/**
 * Returns chars written
 * Copy a string without exceeding the specified limit
 * Will always patch a NULL terminator at the final position
 */
internal i32 ZStr_CopyLimited(const char *source, char *target, zeSize limit)
{
    if (limit < 1) { return 0; }
    i32 written = 0;
    char* targetStart = target;
    while (*source && limit > 0)
    {
        *target++ = *source++;
        --limit;
        ++written;
    }
	*target = '\0';
    return ++written;
}

// decimal or hexadecimal
// negative and positive
// "-54" "12" "0x432146fd" "-0X4AbdC"
internal i32 ZStr_AsciToInt32(const char *str)
{
    i32 sign = 1;
    i32 val = 0;
    char c;
    if (*str == '-')
    {
        sign = -1;
        ++str;
    }

    // hexadecimal
    if (*str == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        // Move past first two characters
        str += 2;
        while (1)
        {
            c = *str;
            ++str;
            if (c >= '0' && c <= '9')
            {
                val = val * 16 + c - '0';
            }
            else if (c >= 'a' && c <= 'f')
            {
                val = val * 16 + c - 'a' + 10;
            }
            else if (c >= 'A' && c <= 'F')
            {
                val = val * 16 + c - 'A' + 10;
            }
            else
            {
                return val * sign;
            }
        }
    }

    // decimal
    while (true)
    {
        c = *str;
        ++str;
        if (c < '0' || c > '9')
        {
            // no numerical character
            return sign * val;
        }
        // '0' char = 48 in asci, so
        // c minus '0' = c minus 48
        // val * 10 moves to next digit
        // then add new value
        val = (val * 10) + (c - '0');
    }
    return val * sign;
}

/////////////////////////////////////////////
// string stack
/////////////////////////////////////////////

#endif // ZE_STRING_UTILS_H
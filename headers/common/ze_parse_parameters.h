#ifndef ZE_PARSE_PARAMETERS
#define ZE_PARSE_PARAMETERS

//////////////////////////////////////////
// More generic command line reading
//////////////////////////////////////////

#include "ze_common.h"

#define ZPG_PARAM_TYPE_FLAG 0
#define ZPG_PARAM_TYPE_STRING 1
#define ZPG_PARAM_TYPE_FUNCTION 2
#define ZPG_PARAM_TYPE_INTEGER 3

typedef i32 (*ze_param_fn)(i32 argc, char** argv, void* data);

struct ZEParam
{
    i32 type;
    char asciChar; // -c data etc
    char* helpText;
    // discriminated union on type field
    union
    {
        i32 flag;
		// Will read an integer from the next token, and use offset bytes
		// to specify where in the target data blob to write the result
        i32 integerOffsetBytes;
		// run a callback
        ze_param_fn func;
    } data;
};

// static i32 g_numParamTypes = 0;
// static ZPGParam g_paramTypes[64];

internal i32 ZE_FindParamIndex(
    const char** params,
    const i32 numTokens,
    const char* query,
    const i32 trailingTokens)
{
    for (i32 i = 0; i < numTokens; ++i)
    {
        const char* txt = params[i];
        if (!ZStr_Compare(txt, query))
        {
            if (trailingTokens <= 0) { return i; }
            i32 remainingTokens = (numTokens) - i;
            if (remainingTokens >= trailingTokens)
            {
                return i;
            }
            return ZE_ERROR_BAD_INDEX;
        }
    }
    return ZE_ERROR_BAD_INDEX;
}

#endif // ZE_PARSE_PARAMETERS
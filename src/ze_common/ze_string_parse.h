#pragma once

#include "ze_common.h"
#include "ze_string_utils.h"

internal void ZE_PrintChars(u8* buf, i32 size, i32 bytesPerLine)
{
	printf("ZN --- Chars (%d) ---\n0:\t", size);
	u8* end = buf + size;
	i32 index = 0;
	while (buf < end)
	{
		printf("%c, ", *buf);
		buf++;
		index++;
		if (index % bytesPerLine == 0)
		{
			printf("\n%d:\t", index);
			//lineChars = 0;
		}
	}
	printf("\n");
}

internal i32 ZE_CountSpecificChar(const char* str, char c)
{
    if (!str) { return 0; }
    i32 i = 0;
    i32 count = 0;
    do
    {
        if (str[i] == c)
        {
            count++;
        }
    } while (str[i++] != '\0');
    return count;
}

/**
 * Find string tokens:
 * > Copies source into dest, placing '\0' at the end of every token
 * > Destination must be at least as long as source!
 * > Returns number of tokens found, up to the max allowed by
 * the passed in results array
 * > Should be okay if destination and token list memory is not cleared.
 * > Send same pointer to source and destination to tokenise in-place.
 */
internal i32 ZE_ReadTokens(const char* source, char* destination, char** tokens, i32 maxTokens)
{
    i32 len = ZE_StrLen(source);
    i32 tokensCount = 0;

    i32 readPos = 0;
    i32 writePos = 0;

    u8 reading = true;
    u8 readingToken = 0;
    while (reading)
    {
        char c = *(source + readPos);
        if (readingToken)
        {
            if (c == ' ')
            {
				// finish token
                *(destination + writePos) = '\0';
				readingToken = 0;
            }
            else if (c == '\0')
            {
				// finish token and complete string
                *(destination + writePos) = '\0';
				readingToken = 0;
                reading = false;
            }
            else
            {
                *(destination + writePos) = c;
            }
            readPos++;
            writePos++;

			// Check if finished and results are full
			if (readingToken == 0 && tokensCount >= maxTokens)
			{
				reading = false;
			}
        }
        else
        {
			// probe forward for a token start
            if (c == ' ' || c == '\t')
            {
                readPos++;
            }
            else if (c == '\0')
            {
                *(destination + writePos) = '\0';
                reading = false;
            }
            else
            {
				// Start token
                readingToken = 1;
                *(destination + writePos) = c;

                tokens[tokensCount++] = (destination + writePos);

                readPos++;
                writePos++;
            }
        }
    }

    return tokensCount;
}

static i32 ZE_CharOkayForFieldName(char c)
{
    // 0...9
    if (c >= 48 && c <= 57) { return YES; }
    // A...Z
    if (c >= 65 && c <= 90) { return YES; }
    // a...z
    if (c >= 97 && c <= 122) { return YES; }
    // underscore
    if (c == 95) { return YES; }
    return NO;
}
#if 0
static void ZE_FindNextToken(char* readStart, char* readEnd, char** tokenStart, char** tokenEnd)
{
    *tokenStart = NULL;
    *tokenEnd = NULL;
    char* cursor = readStart;
    i32 bBuildingToken = NO;
    for (;;)
    {
        char c = *cursor;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            if (bBuildingToken == YES)
            {
                // finish token
                *tokenEnd = cursor;
            }
        }
        else if (c >= 48 && c)
        {

        }
    }
}
#endif
static char* ZE_FindNewLineOrEnd(char* start, char* bufferEnd)
{
    char* cursor = start;
    while(cursor < bufferEnd && (*cursor != '\n' && *cursor != '\r'))
    {
        cursor++;
    }
    return cursor;
}

static char* ZE_RunToNextLine(char* start, char* bufferEnd)
{
    char* cursor = start;
    // find the end of the line
    while(cursor < bufferEnd && (*cursor != '\n'))
    {
        cursor++;
    }
    // move to next char if available.
    if (cursor < bufferEnd)
    {
        cursor++;
    }
    return cursor;
}

static char* ZE_RunPastTerminator(char* start, char* bufferEnd)
{
    char* cursor = start;
    // find the end of the line
    while(cursor < bufferEnd && (*cursor != '\0'))
    {
        cursor++;
    }
    // move to next char if available.
    if (cursor < bufferEnd)
    {
        cursor++;
    }
    return cursor;
}

static char* ZE_ReadToChar(char* start, char c)
{
    while (*start++ != c) { };
    return start;
}
#if 0
static char* ZE_ReadToChars(char* start, char* match)
{
    i32 strLen = ZE_StrLen(start);
    i32 matchLen = ZE_StrLen(match);
    if (strLen < matchLen) { return NULL; }
    i32 remaining = strLen;
    i32 i = 
    while (*start++ != c) { };
    return start;
}
#endif
static i32 ZE_FindFirstCharMatch(char* start, char match)
{
    i32 i = 0;
    for(;;)
    {
        char c = start[i];
        if (c == match) { return i; }
        if (c == '\0') { break; }
        i++;
    }
    return -1;
}

internal char* ZE_RunToNewLine(char* buffer)
{
    u8 reading = true;
    while (reading)
    {
        if (*buffer == '\n' || *buffer == EOF)
        {
            reading = false;
        }
        else
        {
            ++buffer;
        }
    }
    return buffer;
}

internal char* ZE_EatWhiteSpace(char* start)
{
    char* result = start;
    while (*result != NULL && (*result == ' ' || *result == '\t'))
    {
        result++;
    }
    return result;
}

/**
Send a target length of -1 to measure the string. Use a target length > 0
allows for strings with terminators in them
*/
internal i32 ZE_ReplaceChars(char* str, i32 strLen, char* targets, i32 numTargets, char* replacements, i32 numReplacements)
{
    i32 totalReplaced = 0;
    if (str == NULL) { return -1; }
    if (targets == NULL) { return -1; }
    if (replacements == NULL) { return -1; }
    if (numTargets != numReplacements) { return -1; }

    if (strLen < 0)
    {
        strLen = ZE_StrLen(str);
    }
    char* cursor = str;
    for (i32 i = 0; i < strLen; ++i)
    {
        char c = str[i];
        for (i32 j = 0; j < numTargets; ++j)
        {
            if (c == targets[j])
            {
                printf("Replacing %d with %d\n", targets[j], replacements[j]);
                str[i] = replacements[j];
                totalReplaced++;
            }
        }
    }
    return totalReplaced;
}

#if 0
internal i32 ZE_CheckTokensSignature(char** tokens, i32 numTokens, char* signature)
{
	// Check signature is the right length first
	i32 sigLen = ZE_StrLenNoTerminator(signature);
	if (sigLen != numTokens) { return NO; }
	
	for (i32 i = 0; i < numTokens; ++i)
	{
		char* token = tokens[i];
		char sig = signature[i];
		switch (sig)
		{
			case 'T':
			case 't':
			case 'S':
			case 's':
			{
				// Check token is a string... not hard.
				// it exists, it is a string
			} break;
			case 'I':
			case 'i':
			{
				// Check token is an integer (no decimal)
			} break;
		}
	}
	return NO;
}
#endif

internal i32 ZE_StrMeasureLine(char* str)
{
	i32 count = 0;
	while(str)
	{
		char c = *str;
		str++;
		if (c == '\0' || c == '\n' || c == '\r')
		{
			break;
		}
		else if (c == '\t')
		{
			count += 4;
		}
		else
		{
			count++;
		}
	}
	return count;
}

internal void ZE_StrMeasure2D(char* str, i32* x, i32* y)
{
    i32 lineCount = 0;
    i32 curLineLength = 1;
    i32 longestLine = 1;
    char* cursor = str;
    for(;;)
    {
        char c = *cursor;
        if (c == '\n' || c == '\0')
        {
            // finish line
            if (curLineLength > longestLine)
            {
                longestLine = curLineLength;
            }
            curLineLength = 0;
            lineCount++;
        }
        else if (c != '\r')
        {
            curLineLength++;
        }
		else if (c != '\t')
		{
			curLineLength += 4;
		}
        if (c == '\0')
        {
            break;
        }
        else
        {
            cursor++;
        }
    }
    *x = longestLine;
    *y = lineCount;
}

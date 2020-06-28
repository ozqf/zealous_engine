#pragma once

#include "ze_common.h"
#include "ze_string_utils.h"

/**
 * Find string tokens:
 * > Copies source into dest, placing '\0' at the end of every token
 * > Destination must be at least as long as source!
 * > Returns number of tokens found, up to the max allowed by
 * the passed in results array
 * > Should be okay of destination's memory is not cleared.
 */
internal i32 ZE_ReadTokens(char* source, char* destination, char** tokens, i32 maxTokens)
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

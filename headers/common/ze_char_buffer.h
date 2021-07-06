#ifndef ZE_CHAR_BUFFER_H
#define ZE_CHAR_BUFFER_H

#include "ze_common.h"

/**
 * TODO - delete me? Just use a ByteBuffer...
 */
struct CharBuffer
{
    char* chars;
    char* cursor;
    i32 maxLength;

    void Reset()
    {
        cursor = chars;
    }
    
    void Set(char* charArray, i32 numChars)
    {
        chars = charArray;
        cursor = charArray;
        maxLength = numChars;
    }

    CharBuffer StartSubSection()
    {
        // 1 for data 1 for terminator
        ZE_ASSERT(Space() > 2, "No space for substring")
        CharBuffer sub;
        sub.chars = cursor;
        sub.cursor = cursor;
        // -1 so EndSubSection can add null terminator
        sub.maxLength = Space() - 1;
        return sub;
    }

    // Set sub string's length to what has been written
    // so that we don't think we can come back and write into
    // it again
    void EndSubSection(CharBuffer* sub)
    {
        ZE_ASSERT(Space() > 0, "No space for substring terminator")
        *sub->cursor = '\0';
        sub->cursor++;
        // move to end of sub-section
        cursor = sub->cursor;
        // cap subsection off
        sub->maxLength = (sub->cursor - sub->chars);
    }

    i32 Written()
    {
        return this->cursor - this->chars;
    }
    i32 Space()
    {
        return maxLength - (cursor - chars);
    }
};

#endif // ZE_CHAR_BUFFER_H
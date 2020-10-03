#ifndef ZE_BYTE_BUFFER_H
#define ZE_BYTE_BUFFER_H

#include "ze_common.h"
#include "ze_string_utils.h"

#define BUF_COPY(ptrToByteBufferDest, ptrToSourceBytes, numOfBytesInArray) \
{##ptrToByteBufferDest##->cursor += \
    ZE_COPY((u8*)##ptrToSourceBytes##, ##ptrToByteBufferDest##->cursor##, numOfBytesInArray##);}

/**
 * allocate space for an instance of the given struct type in the bytebuffer.
 * if byte buffer has no space, ptr will be null
 * ASSUMES SIZE REQUIRED == sizeof(StructType)
 */
#define ZE_INIT_PTR_IN_PLACE(ptrVariableName, structTypeName, ptrToByteBufferDest) \
structTypeName##* ptrVariableName = NULL; \
if (ptrToByteBufferDest##->Space() >= sizeof(##structTypeName##)) \
{ \
    ptrVariableName = (##structTypeName##*)##ptrToByteBufferDest##->cursor; \
    ptrToByteBufferDest##->cursor += sizeof(##structTypeName##); \
    *##ptrVariableName = {}; \
}


#define ZE_CREATE_STACK_BUF(byteBufferVarName, byteBufferNumBytes) \
u8 byteBufferVarName##Bytes[byteBufferNumBytes]; \
ZEBuffer byteBufferVarName = Buf_FromBytes(byteBufferVarName##Bytes, byteBufferNumBytes);

// TODO: Change this to a char buffer - was dumb to not do that to begin with :/
struct ZEBuffer
{
    u8* start;
    // when writing, advance cursor forward,
    // when reading, read from start to cursor
    u8* cursor;
    i32 capacity;

    i32 Written()
    {
        return this->cursor - this->start;
    }
    i32 Space()
    {
        return capacity - (cursor - start);
    }

    i32 IsValid()
    {
        if (start == NULL) { return NO; }
        if (cursor == NULL) { return NO; }
        if (capacity == 0) { return NO; }
        if (Written() > Space()) { return NO; }
        return YES;
    }

    void Clear(i32 bSetZero)
    {
        if (start != NULL && bSetZero)
        {
            ZE_SET_ZERO(start, capacity);
        }
	    cursor = start;
    }

    i32 CursorOffset()
    {
        return this->cursor - this->start;
    }

    i32 WriteString(const char* str)
    {
        i32 len = ZE_StrLen(str);
        if (this->Space() < len) { return ZE_ERROR_NO_SPACE; }
        this->cursor += ZE_COPY(str, this->cursor, len);
        return ZE_ERROR_NONE;
    }
};

internal i32 Buf_IsValid(ZEBuffer* b)
{
    if (b == NULL) { return NO; }
    return b->IsValid();
}

internal ZEBuffer Buf_FromBytes(u8* ptr, i32 numBytes)
{
    ZE_ASSERT(ptr != NULL, "Buf from bytes - ptr is null");
    ZEBuffer b = {};
    b.start = ptr;
    b.cursor = ptr;
    b.capacity = numBytes;
    return b;
}

internal ZEBuffer Buf_FromMalloc(void* ptr, i32 size)
{
    ZE_ASSERT(ptr != NULL, "Buf from malloc - ptr is null");
    ZEBuffer b = {};
    b.start = (u8*)ptr;
    b.cursor = b.start;
    b.capacity = size;
    return b;
}

/**
 * init another buffer from within the given buffer and return it
 * If the result has a size of 0 then alloc failed
 */
internal ZEBuffer Buf_SubBuffer(ZEBuffer* buf, i32 subBufferSize)
{
    if (buf->Space() < subBufferSize) { return {}; }
    ZEBuffer sub = {};
    sub.start = buf->cursor;
    sub.cursor = sub.start;
    sub.capacity = subBufferSize;

    buf->cursor += subBufferSize;
    return sub;
}

struct ZEDoubleBuffer
{
    i32 swapped;
    ZEBuffer a;
    ZEBuffer b;

    ZEBuffer* GetRead()
    {
        return this->swapped ? &this->b : &this->a;
    }

    ZEBuffer* GetWrite()
    {
        return this->swapped ? &this->a : &this->b;
    }

    void Swap()
    {
        swapped = !swapped;
    }
};

#endif // ZE_BYTE_BUFFER_H
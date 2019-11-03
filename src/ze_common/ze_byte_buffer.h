#ifndef ZE_BYTE_BUFFER_H
#define ZE_BYTE_BUFFER_H

#include "ze_common.h"

#define BUF_COPY(ptrToByteBuffer, ptrToByteArray, numOfBytesInArray) \
{##ptrToByteBuffer##->cursor += \
    ZE_COPY((u8*)##ptrToByteArray##, ##ptrToByteBuffer##->cursor##, numOfBytesInArray##);}

struct ZEByteBuffer
{
    u8* start;
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
};

internal i32 Buf_IsValid(ZEByteBuffer* b)
{
    if (b == NULL) { return NO; }
    return b->IsValid();
}

internal ZEByteBuffer Buf_FromBytes(u8* ptr, i32 numBytes)
{
    ZEByteBuffer b = {};
    b.start = ptr;
    b.cursor = ptr;
    b.capacity = numBytes;
    return b;
}

internal ZEByteBuffer Buf_FromMalloc(void* ptr, i32 size)
{
    ZEByteBuffer b = {};
    b.start = (u8*)ptr;
    b.cursor = b.start;
    b.capacity = size;
    return b;
}

struct ZEDoubleByteBuffer
{
    i32 swapped;
    ZEByteBuffer a;
    ZEByteBuffer b;

    ZEByteBuffer* GetRead()
    {
        return this->swapped ? &this->b : &this->a;
    }

    ZEByteBuffer* GetWrite()
    {
        return this->swapped ? &this->a : &this->b;
    }

    void Swap()
    {
        swapped = !swapped;
    }
};

#endif // ZE_BYTE_BUFFER_H
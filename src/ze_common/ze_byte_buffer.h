#ifndef ZE_BYTE_BUFFER_H
#define ZE_BYTE_BUFFER_H


#include "ze_common.h"

#define BUF_COPY(ptrToByteBuffer, ptrToByteArray, numOfBytesInArray) \
{##ptrToByteBuffer##->cursor += \
    COM_COPY((u8*)##ptrToByteArray##, ##ptrToByteBuffer##->cursor##, numOfBytesInArray##);}

struct ze_byte_buffer
{
    u8* start;
    //u8* end;
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
        return YES;
    }

    void Clear(i32 bSetZero)
    {
        if (start != NULL && bSetZero)
        {
            ZE_SET_ZERO(start, capacity);
        }
	    //end = start;
	    cursor = start;
    }
};

internal i32 Buf_IsValid(ze_byte_buffer* b)
{
    if (b == NULL) { return NO; }
    return b->IsValid();
}

internal ze_byte_buffer Buf_FromBytes(u8* ptr, i32 numBytes)
{
    ze_byte_buffer b = {};
    b.start = ptr;
    b.cursor = ptr;
    b.capacity = numBytes;
    return b;
}

internal ze_byte_buffer Buf_FromMalloc(void* ptr, i32 size)
{
    ze_byte_buffer b = {};
    b.start = (u8*)ptr;
    b.cursor = b.start;
    b.capacity = size;
    return b;
}

#endif // ZE_BYTE_BUFFER_H
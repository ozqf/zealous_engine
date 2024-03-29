#ifndef ZE_BUF_BLOCK_H
#define ZE_BUF_BLOCK_H

#include "../ze_common.h"

#define MEMORY_BLOCKS_PARANOID

struct BufferBlock
{
#ifdef MEMORY_BLOCKS_PARANOID
    i32 sentinel;
#endif
    zeSize size;
    i32 type;
    i32 tag;
};

#ifndef BUF_BLOCK_CAST
#define BUF_BLOCK_CAST(ptr) ((BufferBlock *)##ptr##)
#endif

#ifndef BUF_BLOCK_BEGIN_READ
#define BUF_BLOCK_BEGIN_READ(ptrToBuffer, headerPtrName) \
i8* buf_block_read_ptr = ptrToBuffer->start; \
i8 *buf_block_end_ptr = ptrToBuffer->cursor; \
zErrorCode bufBlockHeaderError = ZE_ERROR_NONE; \
while (buf_block_read_ptr < buf_block_end_ptr) \
{ \
    BufferBlock* headerPtrName = (BufferBlock*)buf_block_read_ptr; \
    bufBlockHeaderError = BufBlock_Validate(headerPtrName); \
    if (bufBlockHeaderError != ZE_ERROR_NONE) \
    { break; } \
    buf_block_read_ptr += headerPtrName->size;
#endif

#ifndef BUF_BLOCK_END_READ
#define BUF_BLOCK_END_READ \
}
#endif

#ifndef BUF_BLOCK_BEGIN_STRUCT
#define BUF_BLOCK_BEGIN_STRUCT(ptrToCreate, structType, ptrToBuffer, bufBlockType) \
ZE_BUF_INIT_PTR_IN_PLACE(ptrToCreate, structType, ptrToBuffer) \
BufBlock_PrepareHeader(&ptrToCreate->header, sizeof(structType), bufBlockType);
#endif

internal void BufBlock_PrepareHeader(BufferBlock *block, zeSize size, i32 type)
{
#ifdef MEMORY_BLOCKS_PARANOID
    block->sentinel = ZE_SENTINEL;
#endif
    block->size = size;
    block->type = type;
}

/**
 * creates a new buffer block header and advances the cursor
 */
internal BufferBlock *BufBlock_Begin(ZEBuffer *buf, zeSize size, i32 type)
{
    if (buf->Space() < size)
    {
        return NULL;
    }
    ZE_BUF_INIT_PTR_IN_PLACE(block, BufferBlock, buf)
#ifdef MEMORY_BLOCKS_PARANOID
    block->sentinel = ZE_SENTINEL;
#endif
    block->size = size;
    block->type = type;
}

internal ErrorCode BufBlock_Validate(BufferBlock *block)
{
    if (block == NULL)
    {
        return ZE_ERROR_NULL_ARGUMENT;
    }
#ifdef MEMORY_BLOCKS_PARANOID
    if (block->sentinel != ZE_SENTINEL)
    {
        return ZE_ERROR_DESERIALISE_FAILED;
    }
#endif
    if (block->size <= 0)
    {
        return ZE_ERROR_BAD_SIZE;
    }
    if (block->type == 0)
    {
        return ZE_ERROR_UNSUPPORTED_OPTION;
    }
    return ZE_ERROR_NONE;
}

internal void BufBlock_Print(ZEBuffer *b)
{
    i8 *read = b->start;
    i8 *end = b->cursor;
    i32 i = 0;
    while (read < end)
    {
        BufferBlock *h = (BufferBlock *)read;
        zErrorCode err = BufBlock_Validate(h);
        if (err == ZE_ERROR_NONE)
        {
            printf("%d: Block type %d, size %zd Bytes\n", i, h->type, h->size);
            read += h->size;
        }
        else
        {
            printf("Block validate error: %d\n", err);
            return;
        }
    }
}

#endif // ZE_BUF_BLOCK_H

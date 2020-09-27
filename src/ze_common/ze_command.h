#ifndef ZE_COMMAND_H
#define ZE_COMMAND_H

#include "ze_common.h"

// a null command is invalid and will cause command validation to fail!
#define ZCMD_TYPE_NULL 0

#define ZCMD_SENTINEL 0xDEADBEEF
#define ZCMD_INVALID_SIZE 0

struct ZECommand
{
	i32 type;
	i32 size;
	i32 sentinel;
};

internal inline i32 ZCmd_Validate(ZECommand* cmd)
{
    if (cmd == NULL)  { return ZE_ERROR_BAD_ARGUMENT; }
    if (cmd->sentinel != ZCMD_SENTINEL) { return ZE_ERROR_DESERIALISE_FAILED; }
    if (cmd->type == 0) { return ZE_ERROR_UNKNOWN_COMMAND; }
    if (cmd->size <= 0) { return ZE_ERROR_BAD_SIZE; }
    return ZE_ERROR_NONE;
}

internal void ZCmd_Prepare(ZECommand* cmd, i32 type, i32 size)
{
    cmd->sentinel = ZCMD_SENTINEL;
    cmd->type = type;
    cmd->size = size;
}

internal void ZCmd_Write(ZECommand* cmd, u8** ptr)
{
    i32 err = ZCmd_Validate(cmd);
    if (err != ZE_ERROR_NONE)
    {
        ZE_ASSERT(0, "ZCmd Write - invalid command")
    }
    ZE_ASSERT(ptr != NULL, "ZCmd Write ptr is null")
    *ptr += ZE_COPY(cmd, *ptr, cmd->size);
}

// Utility macros for reading/writing commands

// TODO: No bounds check here!
// TODO: Assumes cmd size == struct size
#define ZCMD_STRUCT_IN_PLACE(cmdStructType, cmdVarName, cmdType, ptrToByteBuffer) \
cmdStructType* cmdVarName = (cmdStructType*)ptrToByteBuffer->cursor; \
ptrToByteBuffer->cursor += sizeof(cmdStructType); \
ZCmd_Prepare(&cmdVarName->header, cmdType, sizeof(cmdStructType));

// begin stepping through a command buffer
#define ZCMD_BEGIN_ITERATE(ptrToByteBuffer) \
u8* read = ptrToByteBuffer->start; \
u8* end = ptrToByteBuffer->cursor; \
while (read < end) \
{ \
    ZECommand* cmdHeader = (ZECommand*)read; \
    ErrorCode cmdError = ZCmd_Validate(cmdHeader); \
    if (cmdError != ZE_ERROR_NONE) \
    { \
        ZE_ASSERT(0, "Cmd read error"); \
    } \
    read += cmdHeader->size;

// finish stepping block
#define ZCMD_END_ITERATE }

#endif // ZE_COMMAND_H
#pragma once
#if 1
/*
Header only - base for command structs.
*/
#include "../../ze_common/ze_common_full.h"
#include "../app.h"
#include "../../sim/sim.h"

// a null command is invalid and will cause command validation to fail!
#define CMD_TYPE_NULL 0

// 48879?
#define CMD_SENTINEL 0xDEADBEEF
#define CMD_INVALID_SIZE 0

typedef unsigned short CmdSeq;
#define CMD_SEQ_MAX 0xFFFF

// BASE FOR ALL COMMANDS
// All commands MUST have a Command struct as their first member, for
// pointer casting
struct Command
{
    // Type and Size must be set by the implementing command
    u8 type;
    i32 size;
    // for alignment checking
    i32 sentinel;

    // Controls execution time and order
    i32 tick;
    CmdSeq sequence;

    // Server re-transmission control
    // ticks to next transmission
    i8 sendTicks;
    // times sent
    i8 timesSent;
};

internal inline i32 Cmd_WriteSequence(u8* buffer, CmdSeq seq)
{
    return COM_WriteU16(seq, buffer);
}

internal inline CmdSeq Cmd_ReadSequence(u8** buffer)
{
    return COM_ReadU16(buffer);
}

internal inline i32 Cmd_Validate(Command* cmd)
{
    if (cmd == NULL)  { return ZE_ERROR_BAD_ARGUMENT; }
    if (cmd->sentinel != CMD_SENTINEL) { return ZE_ERROR_DESERIALISE_FAILED; }
    if (cmd->type == 0) { return ZE_ERROR_UNKNOWN_COMMAND; }
    if (cmd->size <= 0) { return ZE_ERROR_BAD_SIZE; }
    return ZE_ERROR_NONE;
}
/*
Packets encode their sequence number as a one byte +127 to -128 diff
from a base sequence written at the start of the reliable section

*/
internal i32 Cmd_IsSequenceDiffOkay(i32 diff)
{
    if (diff > 127 || diff < -128) { return NO; }
    return YES;
}

/*internal inline void Cmd_WriteToByteBuffer(ZEBuffer* b, Command* cmd)
{
    ErrorCode err = Cmd_Validate(cmd);
    ZE_ASSERT(!err, "Command failed validation")
    ZE_ASSERT(b->Space() >= cmd->size, "No space for command")
    b->cursor += ZE_COPY(cmd, b->cursor, cmd->size);
}*/

internal void Cmd_Prepare(Command* cmd, i32 tick)
{
    cmd->sentinel = CMD_SENTINEL;
    cmd->tick = tick;
    cmd->type = CMD_TYPE_NULL;
    cmd->size = CMD_INVALID_SIZE;
    cmd->sendTicks = 0;
    cmd->timesSent = 0;
}

////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////
#if 0
internal void Cmd_PrintHeader(Command* header)
{
    printf("Read Cmd type %d. sequence %d, tick %d\n",
		header->type, header->sequence, header->tick);
}

internal void Cmd_PrintBuffer(u8* ptr, i32 numBytes)
{
	printf("\n=== CMD BUFFER (%d bytes) ===\n", numBytes);
    u8* read = ptr;
    u8* end = ptr + numBytes;

    while(read < end)
    {
        Command* header = (Command*)read;
        Assert(Cmd_Validate(header) == ZE_ERROR_NONE);
        read += header->size;
        Cmd_PrintHeader(header);
    }
	printf("  Ptr diff check: %d\n", (read - end));
}
#endif

#endif
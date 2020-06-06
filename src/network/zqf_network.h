#ifndef ZQF_NETWORK_H
#define ZQF_NETWORK_H
/*
ZQF UDP Network module.
> Handles base network message functionality:
	> Command buffers
	> Command Serialisation
	> Command Deserialisation
	> Packet read/write
	> Reliability
*/

#pragma message("ZQF_NETWORK_H")

#include "../ze_common/ze_common.h"

/////////////////////////////////////////
// Flags
/////////////////////////////////////////
#define SERIALISE_FLAG_QUANTISE (1 << 0)

/////////////////////////////////////////
// Error codes
/////////////////////////////////////////
#define NET_COMMAND_ERROR_UNKNOWN -1
#define NET_COMMAND_ERROR_NO_TYPE -2

struct SerialisationInfo;
struct NetCommandType;
struct NetCommand;

typedef unsigned short CmdSeq;

/////////////////////////////////////////
// Function signatures
/////////////////////////////////////////
// Write a command to a packet
typedef void (CmdFn_Serialise)(
	SerialisationInfo* info, NetCommand* source, u8* dest, i32 destCapacity);
// Read a command from a packet
typedef void (CmdFn_Deserialise)(
	SerialisationInfo* info, u8* source, u8* dest, i32 destCapacity);
// Measure a pending command (to make sure there is room in a packet)
typedef i32 (CmdFn_MeasureForSerialise)(
	SerialisationInfo* info, NetCommand* source);

/////////////////////////////////////////
// Data types
/////////////////////////////////////////

struct NetCommandType
{
	i32 typeId;
	char* label;
	// function pointers for this command
	CmdFn_Serialise serialiseFn;
	CmdFn_Deserialise deserialiseFn;
	CmdFn_MeasureForSerialise measureForSerialiseFn;
};

// BASE FOR ALL COMMANDS
// All commands MUST have a Command struct as their first member, for
// pointer casting
struct NetCommand
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

struct SerialisationInfo
{
	i32 flags;
};

/////////////////////////////////////////
// Function exports
/////////////////////////////////////////
extern "C" void Net_RegisterCommand(
	i32 typeId,
	char* label,
	CmdFn_Serialise* serialiseFn,
	CmdFn_Deserialise* deserialiseFn,
	CmdFn_MeasureForSerialise* measureForSerialiseFn
	);
#endif // ZQF_NETWORK_H
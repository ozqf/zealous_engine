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

//#pragma message("ZQF_NETWORK_H")

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_net_types.h"

/////////////////////////////////////////
// Flags
/////////////////////////////////////////
#define SERIALISE_FLAG_QUANTISE (1 << 0)

#define ZN_PACKET_TYPE_REQUEST 251
#define ZN_PACKET_TYPE_CHALLENGE 252
#define ZN_PACKET_TYPE_RESPONSE 253
#define ZN_PACKET_TYPE_PING 254
#define ZN_PACKET_TYPE_DATA 255

#define ZN_PACKET_SIZE 1400

#define ZN_REQUEST_PADDING_BYTES 1000

#define ZN_MAX_PENDING 64
#define ZN_MAX_CONNECTIONS 64

#define ZN_CONN_STATE_DISCONNECTED 0
#define ZN_CONN_STATE_REQUESTING 1
#define ZN_CONN_STATE_CONNECTED 2

#define ZN_FLAG_ACCEPTING_REQUESTS (1 << 0)

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

// specific packet types
struct ZNDataPacket
{
	i32 userId;
	u8* dataPtr;
	i32 dataSize;
};

struct ZNPacketRead
{
	i32 protocol;
	u32 hash;
	u8 type;
	
	u8* payload;
	i32 payloadSize;

	union
	{
		ZNDataPacket dataPacket;
		// value is request, challenge and response
		// depending on type
		u32 value;
	} data;
	
};

struct ZNPacketWrite
{
	// start of packet buffer
	u8* bufPtr;
	// total buffer size
	i32 bufSize;
	// start point of data section
	u8* dataPtr;
	// current write position in the buffer.
	// size of 
	u8* cursor;
};

struct ZNConn
{
	i32 state;
	u32 localSalt;
	u32 remoteSalt;
	ZNetAddress addr;
};

struct ZNPending
{
	// if either salts are zero this pending connection
	// is unused
	u32 clientSalt;
	u32 challengeSalt;
	ZNetAddress addr;
};


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

struct ZNetwork
{
	i32 flags;
	i32 maxConns;
	ZNConn conns[ZN_MAX_CONNECTIONS];
	i32 maxPending;
	ZNPending pending[ZN_MAX_PENDING];
};

/////////////////////////////////////////
// Function exports
/////////////////////////////////////////

extern "C" void ZN_Init(ZNetwork* net);

///////////////////////////////////////
// Read/Write

// generates a random value for Ids
extern "C" i32 ZN_CreateSalt();
extern "C" i32 ZN_Protocol();
extern "C" i32 ZN_PacketHeaderSize();
extern "C" i32 ZN_MessageHeaderSize();
// Defunct:
extern "C" ErrorCode ZN_BuildDataPacket(
	u8* resultBuf, i32 resultCapacity, u32 userId, u8* payload, i32 payloadSize, i32* written);
// Defunct:
extern "C" ErrorCode ZN_BeginPacketRead(
	const u8* buf,
	const i32 size,
	ZNPacketRead* result,
	const i32 bPrintErrors);

extern "C" ZNPacketWrite ZN_BeginPacketWrite(u8* buf, i32 bufferSize);
extern "C" i32 ZN_WrapForTransmission(ZNPacketWrite* packet);
extern "C" void ZN_WritePadBytes(u8* dest, i32 numBytes);

extern "C" i32 ZN_WriteRequestPacket(ZNPacketWrite* writer, u32 userId);
extern "C" i32 ZN_WriteDataPacket(ZNPacketWrite* packet, i32 userId, u8* data, i32 dataSize);

extern "C" i32 ZN_ReadRequest(ZNetwork* net, ZNetAddress addr, i32 requestSalt);

////////////////////////////////////////
// Connections

// acquire an empty connection from the connection pool
extern "C" ZNConn* ZN_GetFreeConn(ZNetwork* net);
// open a connection to the given address and being sending request packets.
extern "C" ZNConn* ZN_RequestConnection(ZNetwork* net, ZNetAddress addr);

////////////////////////////////////////
// debugging
extern "C" void ZN_PrintBytes(u8* buf, i32 size, i32 bytesPerLine);
extern "C" void ZN_PrintChars(u8* buf, i32 size, i32 bytesPerLine);
extern "C" void ZN_PrintConnections(ZNetwork* net);

// Commands
extern "C" void Net_RegisterCommand(
	i32 typeId,
	char* label,
	CmdFn_Serialise* serialiseFn,
	CmdFn_Deserialise* deserialiseFn,
	CmdFn_MeasureForSerialise* measureForSerialiseFn
	);
#endif // ZQF_NETWORK_H
#ifndef COMMAND_REGISTER_H
#define COMMAND_REGISTER_H

#include "zqf_network.h"
#include "../ze_common/ze_common_full.h"
#include "../ze_common/ze_hash.h"

#define ZE_PACKET_TYPE_DATA 255

/*
== Packet Structure ==
> packet header (see below)
> payload - depends on type
	> data - entirely user defined.
	> request - asking for a connection
	> challenge - response to a request
	> response - challenge response
	> handshake - challenge accepted. connection established.
	> ping - keep alive.
*/



/* Packet header:
1 protocol magic chars (4 bytes)
2 hash (4 bytes)
3 type u8 (1 byte)
4 sender Id (4 bytes) - will be zero for request packets
*/
extern "C" i32 ZN_PacketHeaderSize()
{
	return 4 + sizeof(i32) + 1 + sizeof(i32);
}

extern "C" i32 ZN_Protocol()
{
	return 0xFADE;
}

extern "C" ErrorCode ZN_BuildDataPacket(
	u8* resultBuf, i32 resultCapacity, u32 userId,
	u8* payload, i32 payloadSize, i32* written)
{
	*written = 0;
	const i32 headerSize = ZN_PacketHeaderSize();
	if (resultCapacity < payloadSize + headerSize) { return ZE_ERROR_NO_SPACE; }

	u8* write = resultBuf;
	// 1 write protocol
	write += COM_WriteI32(ZN_Protocol(), write);
	const u8* hashCursor = write;
	// 2 - skip over space for hash
	write += sizeof(i32);

	// record start of payload area for hashing
	const u8* payloadStart = write;
	// 3 - packet type
	*write = ZE_PACKET_TYPE_DATA;
	write += 1;
	// 4 - user Id
	write += COM_WriteU32(userId, write);

	const u8* payloadEnd = write + payloadSize;
	const i32 totalSize = payloadEnd - resultBuf;
	//printf("Total Packet Size %d\n", totalSize);

	memcpy(write, payload, payloadSize);
	const u8* end = write + payloadSize;

	// 2 again - calc hash and write it to the reserved space
	//printf("Calc hash for %d bytes\n", payloadSize + 1);
	u32 hash = ZE_Hash_djb2_Fixed((u8*)payloadStart, payloadSize + 5);
	printf("Hashed %d bytes to %d\n", payloadSize + 5, hash);
	//printf("Generated hash %d\n", hash);
	COM_WriteU32(hash, (u8*)hashCursor);
	
	*written = totalSize;
	return ZE_ERROR_NONE;
}

extern "C"
ErrorCode ZN_BeginPacketRead(
	const u8* buf, const i32 size, ZNPacketDescriptor* result, const i32 bPrintErrors)
{
	if (buf == NULL) { return ZE_ERROR_NULL_ARGUMENT; }
	if (result == NULL) { return ZE_ERROR_NULL_ARGUMENT; }

	u8* cursor = (u8*)buf;
	// 1 - protocol
	result->protocol = COM_ReadI32(&cursor);
	if (result->protocol != ZN_Protocol())
	{
		if (bPrintErrors)
		{ printf("Invalid protocol. Expected %d got %d\n", ZN_Protocol(), result->protocol); }
		return ZE_ERROR_DESERIALISE_FAILED;
	}
	// 2 - hash
	result->hash = COM_ReadI32(&cursor);
	result->payloadSize = size - ZN_PacketHeaderSize();
	i32 payloadSizePlusType = result->payloadSize + 5;
	
	u32 calcHash = ZE_Hash_djb2_Fixed(cursor, payloadSizePlusType);
	printf("Hashed %d bytes to %d\n", payloadSizePlusType, calcHash);
	if (result->hash != calcHash)
	{
		printf("hash mismatch - read hash %d calc hash %d\n", result->hash, calcHash);
		printf("Calculated hash for %d bytes\n", payloadSizePlusType);
		return ZE_ERROR_DESERIALISE_FAILED;
	}
	
	// 3 - read type
	result->type = *cursor;
	cursor++;
	// 4 - read user Id
	result->id = COM_ReadU32(&cursor);
	result->payload = cursor;
	//printf("Read packet type %d\n", result->type);
	return ZE_ERROR_NONE;
}

#if 0
static NetCommandType g_cmdTypes[256];
static i32 g_nextCmdType = 0;

static NetCommandType* Net_GetCommandType(i32 typeId)
{
	for (i32 i = 0; i < g_nextCmdType; ++i)
	{
		if (g_cmdTypes[i].typeId = typeId)
		{
			return &g_cmdTypes[i];
		}
	}
	return NULL;
}

extern "C" void Net_RegisterCommand(
	i32 typeId,
	char* label,
	CmdFn_Serialise* serialiseFn,
	CmdFn_Deserialise* deserialiseFn,
	CmdFn_MeasureForSerialise* measureForSerialiseFn
	)
{
	NetCommandType* newType = &g_cmdTypes[g_nextCmdType++];
	newType->typeId = typeId;
	newType->label = label;
	
	// TODO: It doesn't like this assignment?
	// newType->serialiseFn = serialiseFn;
	// newType->deserialiseFn = deserialiseFn;
	// newType->measureForSerialiseFn = measureForSerialiseFn;
}
#endif

#if 0
extern "C" i32 Net_Serialise(NetCommand* cmd, u8* packet, i32 capacity)
{
	NetCommandType* t = Net_GetCommandType(cmd->type);
	if (t == NULL)
	{
		return NET_COMMAND_ERROR_NO_TYPE;
	}
	t->serialiseFn();
	return NET_COMMAND_ERROR_UNKNOWN;
}

extern "C" i32 Net_Deserialise(u8* source,  u8* dest, i32 destCapacity)
{
	return NET_COMMAND_ERROR_UNKNOWN;
}
#endif
#endif // COMMAND_REGISTER_H
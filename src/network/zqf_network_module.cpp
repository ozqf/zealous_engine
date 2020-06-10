#ifndef COMMAND_REGISTER_H
#define COMMAND_REGISTER_H

#include "zqf_network.h"
#include "../ze_common/ze_common_full.h"
#include "../ze_common/ze_hash.h"

#include "zn_connection.h"

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


extern "C" i32 ZN_PacketHeaderSize()
{
	// 1 protocol magic chars (4 bytes)
	// 2 hash (4 bytes)
	return 4 + 4;
}

extern "C" i32 ZN_MessageHeaderSize()
{
	// 1 type u8 (1 byte)
	// 2 sender Id (4 bytes) - will be zero for request packets
	return 1 + 4;
}

extern "C" i32 ZN_CreateSalt()
{
	return ZE_BurtleHashUint(COM_STDRandI32());
}

extern "C" i32 ZN_Protocol()
{
	return 0xFADE;
}

extern "C" void ZN_WritePadBytes(u8* dest, i32 numBytes)
{
	u8* end = dest + numBytes;
	while (dest < end)
	{
		*dest = COM_STDRandI32() % UCHAR_MAX;
		dest++;
	}
}

extern "C"
ErrorCode ZN_BeginPacketRead(
	const u8* buf, const i32 size, ZNPacketDescriptor* result, const i32 bPrintErrors)
{
	if (buf == NULL) { return ZE_ERROR_NULL_ARGUMENT; }
	if (result == NULL) { return ZE_ERROR_NULL_ARGUMENT; }

	u8* cursor = (u8*)buf;
	const u8* end = (u8*)buf + size;
	// 1 - protocol
	result->protocol = COM_ReadI32(&cursor);
	if (result->protocol != ZN_Protocol())
	{
		if (bPrintErrors)
		{ printf("ZN Invalid protocol. Expected 0x%X got 0x%X\n", ZN_Protocol(), result->protocol); }
		return ZE_ERROR_DESERIALISE_FAILED;
	}
	// 2 - hash
	result->hash = COM_ReadI32(&cursor);
	result->payload = cursor;
	result->payloadSize = size - ZN_PacketHeaderSize();
	
	u32 calcHash = ZE_Hash_djb2_Fixed(cursor, result->payloadSize);
	printf("ZN Hashed %d bytes to %d\n", result->payloadSize, calcHash);
	if (result->hash != calcHash)
	{
		printf("ZN hash mismatch - read hash 0x%X calc hash 0x%X\n", result->hash, calcHash);
		printf("ZN Calculated hash for %d bytes\n", result->payloadSize);
		return ZE_ERROR_DESERIALISE_FAILED;
	}
	// 3 - read type
	result->type = *cursor;
	cursor++;

	// 4 - type specific
	switch (result->type)
	{
		case ZN_PACKET_TYPE_DATA:
		{
			ZNDataPacket* dataP = &result->data.dataPacket;
			dataP->userId = COM_ReadU32(&cursor);
			dataP->dataPtr = cursor;
			dataP->dataSize = end - cursor;
		} break;
		case ZN_PACKET_TYPE_REQUEST:
		case ZN_PACKET_TYPE_CHALLENGE:
		case ZN_PACKET_TYPE_RESPONSE:
		result->data.value = COM_ReadU32(&cursor);
		break;
		default:
		{
			printf("ZN Unknown packet type %d\n", result->type);
			return ZE_ERROR_UNSUPPORTED_OPTION;
		};
	}
	return ZE_ERROR_NONE;
}

/**
 * Packet should be ready for transmission after calling this.
 */
extern "C" i32 ZN_WrapForTransmission(ZNPacketWrite* packet)
{
	u8* cursor = packet->bufPtr;
	// 1 Add protocol
	cursor += COM_WriteI32(ZN_Protocol(), cursor);
	// 2 Add hash of packet contents
	i32 dataSize = packet->cursor - packet->dataPtr;
	u32 calcHash = ZE_Hash_djb2_Fixed(packet->dataPtr, dataSize);
	cursor += COM_WriteI32(calcHash, cursor);
	return ZE_ERROR_NONE;
}

extern "C" ZNPacketWrite ZN_BeginPacketWrite(u8* buf, i32 bufferSize)
{
	ZNPacketWrite p = {};
	p.bufPtr = buf;
	p.bufSize = bufferSize;
	// Step data pointer over space for header
	p.dataPtr = buf + ZN_PacketHeaderSize();
	// ready for writing
	p.cursor = p.dataPtr;
	return p;
}

extern "C" void ZN_WriteDataPacket(
	ZNPacketWrite* packet, i32 userId, u8* data, i32 dataSize)
{
	u8* cursor = packet->dataPtr;
	// 1 packet type
	*cursor = ZN_PACKET_TYPE_DATA;
	cursor++;

	// 2 user Id
	cursor += COM_WriteU32(userId, cursor);

	// 3 payload
	cursor += ZE_Copy(cursor, data, dataSize);

	// done - update packet cursor
	packet->cursor = cursor;
}

extern "C" void ZN_WriteRequestPacket(ZNPacketWrite* writer, u32 userId)
{
	u8* cursor = writer->dataPtr;
	*cursor = ZN_PACKET_TYPE_REQUEST;
	cursor += COM_WriteU32(userId, cursor);
	ZN_WritePadBytes(cursor, ZN_REQUEST_PADDING_BYTES);
}

extern "C" void ZN_PrintBytes(u8* buf, i32 size, i32 bytesPerLine)
{
	printf("ZN --- Bytes (%d) ---\n0:\t", size);
	u8* end = buf + size;
	i32 index = 0;
	while (buf < end)
	{
		printf("%02X, ", *buf);
		buf++;
		index++;
		if (index % bytesPerLine == 0)
		{
			printf("\n%d:\t", index);
			//lineChars = 0;
		}
	}
	printf("\n");
}

extern "C" void ZN_PrintChars(u8* buf, i32 size, i32 bytesPerLine)
{
	printf("ZN --- Chars (%d) ---\n0:\t", size);
	u8* end = buf + size;
	i32 index = 0;
	while (buf < end)
	{
		printf("%c, ", *buf);
		buf++;
		index++;
		if (index % bytesPerLine == 0)
		{
			printf("\n%d:\t", index);
			//lineChars = 0;
		}
	}
	printf("\n");
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
#pragma once

#include "../../ze_common/ze_common_full.h"

#define NET_PROTOCOL 0xFADEFADE

#define PACKET_TYPE_DATA 255

// if set this packet has been compressed
#define PACKET_FLAG_QUANTISED (1 << 0)

// TODO: Replace this with a compacted version eventually
struct PacketHeader
{
	// TODO: This stuff
	// Packet header
	//i32 protocol
	//i32 checksum
	//u8 type;
	// private Id of the connection
	i32 id;
	
	// Packet reliability
	u32 packetSequence;
	u32 ackSequence;
	u32 ackBits;
	// settings
	i32 flags;
	// timing
    i32 transmissionTickNumber;
	timeFloat transmissionTime;
    i32 lastReceivedTickNumber;
	// payload
    u16 numReliableBytes;
    u16 numUnreliableBytes;
};

// Fully unpacked descriptor for use when reading packets
struct PacketDescriptor
{
    u8* ptr;
	u8* cursor;
    i32 size;
	
	// if 0, no data
	// num bytes is offset gap to unreliable section - sync check size.
	i32 reliableOffset;
	i32 deserialiseCheck;
	// if 0, no data.
	// num bytes is size of packet - offset.
	i32 unreliableOffset;
	
	PacketHeader header;
	ZNetAddress sender;
	// base sim tick for the following commands
	i32 unreliableHeader;

	i32 Space()
	{
		return size - (cursor - ptr);
	}
};

internal i32 Packet_GetHeaderSize()
{
    return sizeof(PacketHeader);
}

internal void Packet_ReadHeader(u8* ptr, PacketHeader* h)
{
    *h = *(PacketHeader*)ptr;
}

// Returns bytes written
internal i32 Packet_WriteHeader(u8* ptr, PacketHeader* h)
{
    *(PacketHeader*)ptr = *h;
	return sizeof(PacketHeader);
}

internal void Packet_StartWrite(
	ZEBuffer* packet,
	i32 privateId,
	i32 packetSequence,
	u32 ackSequence,
	u32 ackBits,
	i32 simFrame,
	timeFloat time,
	i32 lastReceivedTickNumber,
	i32 flags)
{
	// create struct in-place
	PacketHeader* h = (PacketHeader*)packet->start;
	*h = {};
	h->id = privateId;
	h->packetSequence = packetSequence;
	h->ackSequence = ackSequence;
	h->ackBits = ackBits;
	h->transmissionTickNumber = simFrame;
	h->transmissionTime = time;
	h->lastReceivedTickNumber = lastReceivedTickNumber;
	h->numReliableBytes = 0;
	h->numUnreliableBytes = 0;	
	h->flags = flags;
	packet->cursor = packet->start + Packet_GetHeaderSize();
}

internal void Packet_FinishWrite(
	ZEBuffer* packet,
	i32 numReliableBytes,
	i32 numUnreliableBytes)
{
	PacketHeader* h = (PacketHeader*)packet->start;
	h->numReliableBytes = (u16)numReliableBytes;
	h->numUnreliableBytes = (u16)numUnreliableBytes;
}

/*
> PacketHeader
> i32 reliable header
> reliable bytes
> u32 sentinel
> i32 unreliable header
> unreliable bytes

*/

internal i32 Packet_InitDescriptor(
	PacketDescriptor* descriptor,
	u8* buf,
	i32 numBytes,
	ZNetAddress addr)
{
	//printf("=== Build packet descriptor (%d bytes)===\n", numBytes);
	//COM_PrintBytesHex(buf, numBytes, 16);
	*descriptor = {};
	descriptor->ptr = buf;
	descriptor->cursor = buf;
	descriptor->size = numBytes;
	descriptor->sender = addr;
	
	// copy out struct
	PacketHeader* h = (PacketHeader*)buf;
	descriptor->header = *h;
	descriptor->reliableOffset = (i32)(Packet_GetHeaderSize());
	descriptor->unreliableOffset =
		descriptor->reliableOffset +
		descriptor->header.numReliableBytes +
		sizeof(u32); // Sentinel
	
	//descriptor->flags = h->flags;
	//printf("Reliable bytes: %d\n", packet->numReliableBytes);
	//printf("Unreliable bytes: %d\n", packet->numUnreliableBytes);
	
	i32 syncOffset  = (descriptor->reliableOffset + descriptor->header.numReliableBytes);
	i32* syncCheckCursor = (i32*)(buf + syncOffset);
	descriptor->deserialiseCheck = *syncCheckCursor;
	if (descriptor->deserialiseCheck != ZE_SENTINEL)
	{
		//*descriptor = {};
		printf("Deserialise check failed! Expected %X got %X\n",
			ZE_SENTINEL, descriptor->deserialiseCheck);
		return ZE_ERROR_DESERIALISE_FAILED;
	}

	return ZE_ERROR_NONE;
}

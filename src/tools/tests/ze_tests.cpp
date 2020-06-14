#include <stdio.h>

#include "../../ze_common/ze_common_full.h"
#include "../../ze_common/ze_blob_store.h"
#include "../../ze_common/ze_random_table.h"

#include "../../network/zqf_network.h"

#include "test_delta_introspection.h"

struct TestBlobObj
{
	i32 foo;
	i32 bar;
	f32 vec[3];
};

internal void TestBlobStore()
{
	printf("--- Test Blob store ---\n");
	ZEBlobStore store;
	ErrorCode err = ZE_InitBlobStore(&store, 32, sizeof(TestBlobObj), 0);
	printf("\tcapacity %d, user blob size %d\n",
		store.m_array->m_maxBlobs, store.m_array->m_blobUserSize);
	printf("--- Salt generator ---\n");
	u32 saltA = ZN_CreateSalt();
	u32 saltB = ZN_CreateSalt();
	printf("Salts A: %d, B: %d\n", saltA, saltB);
	printf("Xor: %d\n", saltA ^ saltB);

}

internal void Test_NetworkUtils()
{
	printf("=== Test Packet Utils ===\n");
	printf("Net protocol: %d / 0x%X\n", ZN_Protocol(), ZN_Protocol());
	printf("Sizeof header %d / 0x%X\n", ZN_PacketHeaderSize(), ZN_PacketHeaderSize());
	printf("Data packet type: %d / 0x%X\n", ZN_PACKET_TYPE_DATA, ZN_PACKET_TYPE_DATA);
	// u8 buf[128];
	// ZN_WritePadBytes(buf, 128);
	// ZN_PrintBytes(buf, 128, 16);
}

internal i32 Test_PacketReadWrite()
{
	COM_STDSeedRandom();
	printf("=== Test Packet Read/Write ===\n");
	ErrorCode err = ZE_ERROR_NONE;
	
	///////////////////////
	// test write
	//////////////////////

	///////////////////////
	// create a payload and header info
	char* data = "This is some data for a packet.";
	i32 dataSize = strnlen_s(data, 99999);
	printf("Sizeof data for packet: %d\n", dataSize);
	u32 testId = 0xDEADBEEF;

	///////////////////////
	// build packet
	// create a packet buffer and build packet
	u8 buf[ZN_PACKET_SIZE];
	ZE_SET_ZERO(buf, ZN_PACKET_SIZE);
	i32 written;
	/* // Old
	i32 err = ZN_BuildDataPacket(buf, 1024, testId, (u8*)data, dataSize, &written);
	if (err != 0)
	{
		printf("FAIL: Packet write result: %d. written %d, expected %d\n",
			err, written, dataSize + ZN_PacketHeaderSize());
		return;
	}*/
	// New
	ZNPacketWrite writer = ZN_BeginPacketWrite(buf, ZN_PACKET_SIZE);
	ZN_WriteDataPacket(&writer, testId, (u8*)data, dataSize);
	written = writer.cursor - writer.bufPtr;
	// finish 
	ZN_WrapForTransmission(&writer);

	printf("\n");
	printf("--- Packet write result ---\n");
	ZN_PrintBytes(writer.bufPtr, written, 16);
	printf("\n");
	ZN_PrintChars(writer.bufPtr, written, 16);

	///////////////////////
	// test read
	//////////////////////
	ZNPacketRead descriptor;
	err = ZN_BeginPacketRead(buf, written, &descriptor, YES);
	if (err != 0)
	{
		printf("\nFAIL: Error %d from begin packet read\n\n", err);
		return ZE_ERROR_TEST_FAILED;
	}
	printf("Descriptor: Protocol 0x%X, Hash %d, type %d\nDatasize %dB\n",
		descriptor.protocol, descriptor.hash, descriptor.type, descriptor.payloadSize);
	
	u8 dataResultBuf[ZN_PACKET_SIZE];
	ZE_SET_ZERO(dataResultBuf, ZN_PACKET_SIZE);

	if (descriptor.type == ZN_PACKET_TYPE_DATA)
	{
		ZNDataPacket dataP = descriptor.data.dataPacket;
		memcpy(dataResultBuf, dataP.dataPtr, dataP.dataSize);
		if (ZE_CompareStrings((const char*)data, (const char*)dataResultBuf) != 0)
		{
			printf("\nFAIL: Data payload mismatch!\n\n");
			printf("\"%s\" expected\n", data);
			printf("\"%s\" read\n", dataResultBuf);
			return ZE_ERROR_TEST_FAILED;
		}
		printf("Data Packet - Payload (%d bytes): %s\n", dataP.dataSize, dataResultBuf);
	}
	return ZE_ERROR_NONE;
}

internal i32 Test_CreateConnection()
{
	ErrorCode err = ZE_ERROR_NONE;
	///////////////////////
	// connection establishment
	//////////////////////
	ZNetwork* client = (ZNetwork*)malloc(sizeof(ZNetwork));
	ZNetwork* server = (ZNetwork*)malloc(sizeof(ZNetwork));
	server->flags |= ZN_FLAG_ACCEPTING_REQUESTS;
	ZN_Init(client);
	ZN_Init(server);

	ZNetAddress clientAddr = {};
	clientAddr.port = 666;
	ZNConn* conn = ZN_RequestConnection(client, clientAddr);
	printf("Opened connection request, local salt %d\n",
		conn->localSalt);
	ZN_PrintConnections(client);

	u8 buf[ZN_PACKET_SIZE];
	ZNPacketWrite writer;
	ZNPacketRead reader;

	///////////////////////////////
	// Request
	printf("\nCL Write Request\n");
	writer = ZN_BeginPacketWrite(buf, ZN_PACKET_SIZE);
	i32 written = ZN_WriteRequestPacket(&writer, conn->localSalt);
	printf("Wrote request (%dB)\n", writer.cursor - writer.bufPtr);
	ZN_WrapForTransmission(&writer);

	ZN_BeginPacketRead(buf, written, &reader, YES);
	u32 challenge = 0;
	ZN_ReadRequest(server, clientAddr, reader.data.value, &challenge);
	ZN_PrintConnections(server);
	///////////////////////////////
	// Challenge
	// sv write
	printf("\nSV - Write challenge\n");
	writer = ZN_BeginPacketWrite(buf, ZN_PACKET_SIZE);
	written = ZN_WriteChallengePacket(&writer, challenge);
	ZN_WrapForTransmission(&writer);

	// cl read
	printf("\nCL - Read challenge\n");
	ZN_PrintBytes(writer.bufPtr, written, 16);
	err = ZN_BeginPacketRead(buf, written, &reader, YES);
	if (err != ZE_ERROR_NONE)
	{
		printf("FAIL code %d\n", err);
		return ZE_ERROR_TEST_FAILED;
	}
	if (reader.type != ZN_PACKET_TYPE_CHALLENGE)
	{
		printf("FAIL! - not a challenge packet - type %02X expected %02X\n",
			reader.type, ZN_PACKET_TYPE_CHALLENGE);
		return 1;
	}
	u32 response = 0;
	err = ZN_ReadChallenge(client, clientAddr, reader.data.value, &response);
	
	///////////////////////////////
	// done
	free(client);
	free(server);
	return ZE_ERROR_NONE;
}

internal void NetworkUnitTests()
{
	Test_NetworkUtils();
	Test_PacketReadWrite();
	Test_CreateConnection();
}

extern "C" void ZETests_Run()
{
	printf("=== ZE tests ===\n");
	TestBlobStore();
	NetworkUnitTests();
	Test_Introspection();
}

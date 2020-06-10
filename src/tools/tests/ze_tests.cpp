#include <stdio.h>

#include "../../ze_common/ze_common_full.h"
#include "../../ze_common/ze_blob_store.h"
#include "../../ze_common/ze_random_table.h"

#include "../../network/zqf_network.h"

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
}

internal void Test_NetworkUtils()
{
	printf("=== Test Packet Utils ===\n");
	printf("Net protocol: %d / 0x%X\n", ZN_Protocol(), ZN_Protocol());
	printf("Sizeof header %d / 0x%X\n", ZN_PacketHeaderSize(), ZN_PacketHeaderSize());
	printf("Data packet type: %d / 0x%X\n", ZN_PACKET_TYPE_DATA, ZN_PACKET_TYPE_DATA);
	u8 buf[128];
	ZN_WritePadBytes(buf, 128);
	ZN_PrintBytes(buf, 128, 16);
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
	ZNPacketDescriptor descriptor;
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
	///////////////////////
	// connection establishment
	//////////////////////
	printf("--- Salt generator ---\n");
	u32 saltA = ZN_CreateSalt();
	u32 saltB = ZN_CreateSalt();
	printf("Salts A: %d, B: %d\n", saltA, saltB);
	printf("Xor: %d\n", saltA ^ saltB);

	ZNetAddress addr = {};
	addr.port = 666;
	ZNConn* conn = ZN_RequestConnection(addr);
	printf("Opened connection request, local salt %d\n",
		conn->localSalt);
	
	
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
}

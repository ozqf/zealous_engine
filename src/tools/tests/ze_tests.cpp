#include <stdio.h>

#include "../../ze_common/ze_common.h"
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

internal void NetworkUnitTests()
{
	COM_STDSeedRandom();
	printf("--- Test Packets ---\n");
	printf("Net protocol: %d\n", ZN_Protocol());
	printf("Sizeof header %d\n", ZN_PacketHeaderSize());

	//char buf[512];
	//printf("Size of data block %d\n", sizeof(buf));

	// create a payload
	char* data = "This is some data for a packet.";
	i32 dataSize = strnlen_s(data, 99999);
	printf("Size of data for packet: %d\n", dataSize);

	// create a packet buffer and build packet
	u8 buf[1024];
	u32 testId = 0xDEADBEEF;
	i32 written;
	i32 err = ZN_BuildDataPacket(buf, 1024, testId, (u8*)data, dataSize, &written);
	if (err != 0)
	{
		printf("FAIL: Packet write result: %d. written %d, expected %d\n",
			err, written, dataSize + ZN_PacketHeaderSize());
		return;
	}
	
	ZNPacketDescriptor descriptor;
	err = ZN_BeginPacketRead(buf, written, &descriptor, YES);
	if (err != 0)
	{
		printf("FAIL: Error %d from begin packet read\n", err);
		return;
	}
	printf("Descriptor:Protocol 0x%X, Hash %d, type %d User Id %d\nDatasize %d\n",
		descriptor.protocol, descriptor.hash, descriptor.type, descriptor.id, descriptor.payloadSize);
	
	u8 resultBuf[1024];
	memcpy(resultBuf, descriptor.payload, descriptor.payloadSize);
	printf("Payload: %s\n", resultBuf);

	///////////////////////
	// connection establishment
	//////////////////////
	ZNetAddress addr = {};
	addr.port = 666;
	ZNConn* conn = ZN_RequestConnection(addr);
	printf("Opened connection request, local salt %d\n",
		conn->localSalt);
	
	u32 saltA = ZN_CreateSalt();
	u32 saltB = ZN_CreateSalt();
	printf("Salts A: %d, B: %d\n", saltA, saltB);
	printf("Xor: %d\n", saltA ^ saltB);
}

extern "C" void ZETests_Run()
{
	printf("=== ZE tests ===\n");
	TestBlobStore();
	NetworkUnitTests();
}

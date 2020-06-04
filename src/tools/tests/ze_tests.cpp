#include <stdio.h>

#include "../../ze_common/ze_common.h"
#include "../../ze_common/ze_blob_store.h"

struct TestBlobObj
{
	i32 foo;
	i32 bar;
	f32 vec[3];
};

internal void TestBlobStore()
{
	printf("Test Blob store\n");
	ZEBlobStore store;
	ErrorCode err = ZE_InitBlobStore(&store, 32, sizeof(TestBlobObj), 0);
	printf("\tcapacity %d, user blob size %d\n",
		store.m_array->m_maxBlobs, store.m_array->m_blobUserSize);
}

extern "C" void ZETests_Run()
{
	printf("=== ZE tests ===\n");
	TestBlobStore();
}

#include "zqf_network.h"

internal ZNPending* ZN_FindPending(ZNetwork* net, ZNetAddress addr, u32 salt)
{
	ZNPending* firstFree = NULL;
	for (i32 i = 0; i < net->maxPending; ++i)
	{
		ZNPending* item = &net->pending[i];
		if (item->clientSalt == salt)
		{
			return item;
		}
		else if (firstFree == NULL && item->clientSalt == 0)
		{
			firstFree = item;
		}
	}
	// no pending was found, create one!
	firstFree->challengeSalt = ZN_CreateSalt();
	firstFree->clientSalt = salt;
	firstFree->addr = addr;
	printf("Init pending for %d - challenge %d\n",
		firstFree->clientSalt, firstFree->clientSalt);
	return firstFree;
}

/**
 * What if two requests come in for the same client?
 * What if a request comes in for a client who was just accepted?
 * 	 (delayed packet)
 */
extern "C" i32 ZN_ReadRequest(
	ZNetwork* net, ZNetAddress addr, i32 requestSalt, u32* challenge)
{
	if ((net->flags & ZN_FLAG_ACCEPTING_REQUESTS)  != 0)
	{
		printf("FAIL: Network is not accepting requests\n");
		return ZE_ERROR_UNSUPPORTED_OPTION;
	}
	ZNConn* conn = ZN_FindConnByRemoteId(net, requestSalt);
	if (conn != NULL)
	{
		printf("ZN %d is already connected!\n", requestSalt);
		return ZE_ERROR_NONE;
	}
	printf("ZN Opening pending request for %d\n", requestSalt);
	ZNPending* pending = ZN_FindPending(net, addr, requestSalt);
	*challenge = pending->challengeSalt;
	return ZE_ERROR_NONE;
}

extern "C" i32 ZN_ReadChallenge(ZNetwork* net, ZNetAddress addr, u32 challenge)
{
	return ZE_ERROR_NONE;
}

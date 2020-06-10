
#include "zqf_network.h"

//internal ZNPending g_pending[ZN_MAX_PENDING];

internal ZNPending* ZN_FindPending(ZNetwork* net, ZNetAddress addr, u32 salt)
{
	ZNPending* firstFree = NULL;
	for (i32 i = 0; i < net->maxPending; ++i)
	{
		ZNPending* item = &net->pending[i];
		if (item->clientSalt == item->challengeSalt)
		{
			return item;
		}
		else if (firstFree == NULL && item->clientSalt == 0)
		{
			firstFree = item;
			firstFree->challengeSalt = 0;
			firstFree->addr = addr;
		}
	}
	return firstFree;
}

/**
 * What if two requests come in for the same client?
 * What if a request comes in for a client who was just accepted?
 * 	 (delayed packet)
 */
extern "C" i32 ZN_ReadRequest(ZNetwork* net, ZNetAddress addr, i32 requestSalt)
{
	ZNConn* conn = ZN_FindConnByRemoteId(net, requestSalt);
	return ZE_ERROR_NONE;
}

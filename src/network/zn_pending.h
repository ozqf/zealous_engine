
#include "zqf_network.h"

#define ZN_MAX_PENDING 64

internal ZNPending g_pending[ZN_MAX_PENDING];

internal ZNPending* ZN_FindPending(ZNetAddress addr, u32 salt)
{
	ZNPending* firstFree = NULL;
	for (i32 i = 0; i < ZN_MAX_PENDING; ++i)
	{
		ZNPending* item = &g_pending[i];
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

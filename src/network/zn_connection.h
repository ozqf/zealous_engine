#include "zqf_network.h"

extern "C"
ZNConn* ZN_RequestConnection(ZNetwork* net, ZNetAddress addr)
{
	ZNConn* conn = ZN_GetFreeConn(net);
	if (conn == NULL) { return NULL; }
	conn->state = ZN_CONN_STATE_REQUESTING;
	conn->localSalt = ZN_CreateSalt();
	conn->remoteSalt = 0;
	conn->addr = addr;
	return conn;
}

extern "C"
ZNConn* ZN_GetFreeConn(ZNetwork* net)
{
	for (i32 i = 0; i < net->maxConns; ++i)
	{
		if (net->conns[i].state == ZN_CONN_STATE_DISCONNECTED)
		{
			return &net->conns[i];
		}
	}
	return NULL;
}

internal ZNConn* ZN_FindConnByRemoteId(ZNetwork* net, u32 remoteId)
{
	for (i32 i = 0; i < ZN_MAX_CONNECTIONS; ++i)
	{
		ZNConn* conn = &net->conns[i];
		if (conn->state != ZN_CONN_STATE_DISCONNECTED
			&& conn->remoteSalt == remoteId)
		{
			return conn;
		}
	}
	return NULL;
}

internal ZNConn* ZN_FindConnBySaltXor(ZNetwork* net, u32 xor)
{
	for (i32 i = 0; i < ZN_MAX_CONNECTIONS; ++i)
	{
		ZNConn* conn = &net->conns[i];
		u32 saltXor = conn->localSalt ^ conn->remoteSalt;
		if (conn->state != ZN_CONN_STATE_DISCONNECTED
			&& saltXor == xor)
		{
			return conn;
		}
	}
	return NULL;
}

extern "C" void ZN_PrintConnections(ZNetwork* net)
{
	printf("--- ZN connections (%d max) ---\n", ZN_MAX_CONNECTIONS);
	for (i32 i = 0; i < net->maxConns; ++i)
	{
		ZNConn* conn = &net->conns[i];
		if (conn->state == ZN_CONN_STATE_DISCONNECTED)
		{ continue; }
		printf("%d: local %d remote %d. state %d addr %d.%d.%d.%d:%d\n",
			i,
			conn->localSalt,
			conn->remoteSalt,
			conn->state,
			conn->addr.ip4Bytes[0],
			conn->addr.ip4Bytes[1],
			conn->addr.ip4Bytes[2],
			conn->addr.ip4Bytes[3],
			conn->addr.port
		);
	}
	printf("--- ZN Pending (%d max) ---\n", ZN_MAX_PENDING);
	for (i32 i = 0; i < net->maxPending; ++i)
	{
		ZNPending* p = &net->pending[i];
		if (p->clientSalt == 0)
		{ continue; }
		printf("%d: challenge %d client %d. addr %d.%d.%d.%d:%d\n",
			i,
			p->challengeSalt,
			p->clientSalt,
			p->addr.ip4Bytes[0],
			p->addr.ip4Bytes[1],
			p->addr.ip4Bytes[2],
			p->addr.ip4Bytes[3],
			p->addr.port
		);
	}
}

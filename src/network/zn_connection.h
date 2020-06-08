
#include "zqf_network.h"

#define ZN_MAX_CONNECTIONS 64

#define ZN_CONN_STATE_DISCONNECTED 0
#define ZN_CONN_STATE_REQUESTING 1
#define ZN_CONN_STATE_CONNECTED 2

internal ZNConn g_connections[ZN_MAX_CONNECTIONS];

extern "C"
ZNConn* ZN_RequestConnection(ZNetAddress addr)
{
	ZNConn* conn = ZN_GetFreeConn();
	if (conn == NULL) { return NULL; }
	conn->state = ZN_CONN_STATE_REQUESTING;
	conn->localSalt = ZN_CreateSalt();
	conn->remoteSalt = 0;
	conn->addr = addr;
	return conn;
}

extern "C"
ZNConn* ZN_GetFreeConn()
{
	for (i32 i = 0; i < ZN_MAX_CONNECTIONS; ++i)
	{
		if (g_connections[i].state == ZN_CONN_STATE_DISCONNECTED)
		{
			return &g_connections[i];
		}
	}
	return NULL;
}

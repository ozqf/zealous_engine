/*
Handles connection requests from clients
*/
#ifndef SERVER_CONNECT_H
#define SERVER_CONNECT_H

#include "server_internal.h"

struct SV_PendingConnection
{
	ZNetAddress addr;
	i32 requestToken;
	i32 challengeToken;
};

// client requests that have been challeneged.
// we are awaiting a response.
internal SV_PendingConnection g_pendingConnections[64];

internal SV_PendingConnection* SVConnection_FindPending(ZNetAddress* addr)
{
	return NULL;
}

internal i32 SVConnection_CreateChallengeToken()
{
	return 0xDEADBEEF;
}

internal void SVConnection_SendChallenge(SV_PendingConnection* conn)
{
	APP_PRINT(64, "SV Challenge %d to %d.%d.%d.%d:%d\n",
		conn->challengeToken,
		conn->addr.ip4Bytes[0],
		conn->addr.ip4Bytes[1],
		conn->addr.ip4Bytes[2],
		conn->addr.ip4Bytes[3],
		conn->addr.port
	);
}

internal void SVConnection_HandleRequest(PacketDescriptor* p)
{
	ZNetAddress* addr = &p->sender;
	printf("SV - connection request from %d.%d.%d.%d:%d\n",
		addr->ip4Bytes[0],
		addr->ip4Bytes[1],
		addr->ip4Bytes[2],
		addr->ip4Bytes[3],
		addr->port);
	printf("\tSV reliable: %dB unreliable: %dB\n",
		p->header.numReliableBytes,
		p->header.numUnreliableBytes
	);
	// look for a join request command
	// if the first thing isn't a request, then screw it.
	u8* cursor = p->ptr + p->reliableOffset;
	Command* h = (Command*)cursor;
	if (h->type == CMD_TYPE_C2S_JOIN_REQUEST)
	{
		C2S_JoinRequest* req = (C2S_JoinRequest*)h;
		// do
		/*
		> look for a pending request. If found, resend challenge
		> If not, create a pending connection and send challenge
		*/
		SV_PendingConnection* pending = SVConnection_FindPending(addr);
		if (pending != NULL)
		{
			SVConnection_SendChallenge(pending);
			return;
		}
		else
		{
			SV_PendingConnection conn = {};
			conn.requestToken = req->token;
			conn.challengeToken = SVConnection_CreateChallengeToken();
			conn.addr = req->addr;
			SVConnection_SendChallenge(&conn);
		}
		
	}
	else
	{
		APP_PRINT(64, "SV - packet had no request command - ignored\n");
	}
}

#endif // SERVER_CONNECT_H
#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include "client_internal.h"

internal i32 g_requestToken = 0xDEADBEEF;

internal void CL_StartRequesting()
{
	C2S_JoinRequest cmd;
	Cmd_InitJoinRequest(&cmd, 1, g_requestToken, g_serverAddress);
	Stream_EnqueueOutput(&g_reliableStream, &cmd.header);
}

internal void CL_SendRequest()
{
	printf("CL - Sending request!\n");
	u8 buf[APP_MAX_PACKET_SIZE];
	ZEByteBuffer packet = Buf_FromBytes(buf, APP_MAX_PACKET_SIZE);
	CL_WritePacket(NULL, g_elapsed, NULL);

	App_SendTo(g_udpSocketId, &g_serverAddress, buf, packet.Written());
}

internal void CL_TickRequesting(timeFloat delta, i64 platformFrame)
{
	if (g_requestTick <= 0)
	{
		g_requestTick = (timeFloat)4;
		CL_SendRequest();
	}
	else
	{
		g_requestTick -= (f32)delta;
	}
}

#endif // CLIENT_CONNECTION_H
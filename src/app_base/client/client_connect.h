#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include "client_internal.h"

internal i32 g_requestToken = 0xDEADBEEF;

const timeFloat g_requestRetryTime = 2;

internal void CL_StartRequesting()
{
	// add a join request comment to output stream
	C2S_JoinRequest cmd;
	Cmd_InitJoinRequest(&cmd, 1, g_requestToken, g_serverAddress);
	Stream_EnqueueOutput(&g_reliableStream, &cmd.header);
}

internal void CL_TickRequesting(timeFloat delta, i64 platformFrame)
{
	// Periodically send the enqueued join request
	if (g_requestTick <= 0)
	{
		g_requestTick = g_requestRetryTime;
		printf("CL - Sending request!\n");
		CL_WriteAndSendPacketFromStreams(NULL, g_elapsed, NULL);
	}
	else
	{
		g_requestTick -= (f32)delta;
	}
}

#endif // CLIENT_CONNECTION_H
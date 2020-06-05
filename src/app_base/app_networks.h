#pragma once

void App_SendTo(i32 socketIndex, ZNetAddress* addr, u8* data, i32 dataSize)
{
	APP_PRINT(64, "App - send %dB on socket %d\n", dataSize, socketIndex);
	g_loopbackSocket.SendPacket(socketIndex, addr, data, dataSize);
	// send to debug port
	if (g_debugSocket >= 0)
	{
		g_platform.Send(g_debugSocket, ZE_LocalHost(ZE_MONITOR_PORT), data, dataSize);
	}
	#if 0
    if (addr->port == APP_CLIENT_LOOPBACK_PORT)
    {
		Sys_WritePacketEvent(g_clientLoopback.GetWrite(), socketIndex, addr, data, dataSize);
    }
    else if(addr->port == APP_SERVER_LOOPBACK_PORT)
    {
        Sys_WritePacketEvent(g_serverLoopback.GetWrite(), socketIndex, addr, data, dataSize);
    }
    else
    {
		// TODO: Call Socket for remote sends!
		ILLEGAL_CODE_PATH
    }
	#endif
}

internal void App_UpdateLoopbackSocket(FakeSocket* socket, timeFloat deltaTime)
{
	socket->Tick(deltaTime);
	
	i32 socketIndex;
	u8* data;
	i32 dataSize;
	ZNetAddress addr;
	for(;;)
	{
		socket->Read(&socketIndex, &data, &dataSize, &addr);
		if (data == NULL) { break; }
		
		if (addr.port == APP_CLIENT_LOOPBACK_PORT)
		{
			ZE_ASSERT(CL_IsRunning(), "Not running a client!")
			APP_PRINT(64, "App - got %dB from CL loopback\n", dataSize);
			Sys_WritePacketEvent(g_clientLoopback.GetWrite(), socketIndex, &addr, data, dataSize);
		}
		else if(addr.port == APP_SERVER_LOOPBACK_PORT)
		{
			ZE_ASSERT(SV_IsRunning(), "Not running a server!")
			APP_PRINT(64, "App - got %dB from SV loopback\n", dataSize);
			Sys_WritePacketEvent(g_serverLoopback.GetWrite(), socketIndex, &addr, data, dataSize);
		}
		else
		{
			// TODO: Call Socket for remote sends!
			ILLEGAL_CODE_PATH
		}
	}
}

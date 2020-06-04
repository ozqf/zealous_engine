#pragma once

void App_SendTo(i32 socketIndex, ZNetAddress* addr, u8* data, i32 dataSize)
{
	g_loopbackSocket.SendPacket(socketIndex, addr, data, dataSize);
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
			Sys_WritePacketEvent(g_clientLoopback.GetWrite(), socketIndex, &addr, data, dataSize);
		}
		else if(addr.port == APP_SERVER_LOOPBACK_PORT)
		{
			ZE_ASSERT(SV_IsRunning(), "Not running a server!")
			Sys_WritePacketEvent(g_serverLoopback.GetWrite(), socketIndex, &addr, data, dataSize);
		}
		else
		{
			// TODO: Call Socket for remote sends!
			ILLEGAL_CODE_PATH
		}
	}
}

#ifndef ZT_MONITOR_CPP
#define ZT_MONITOR_CPP

#include "zt_monitor.h"

#include "../win_platform/ze_win_socket.h"
#include "windows.h"

internal i32 g_socket;
internal u16 g_port;

internal u8 g_stagingBuf[KiloBytes(64)];

static void ZMonitor_Loop()
{
	for(;;)
	{
		Sleep(33);
		u8* buf = g_stagingBuf;
		ZNetAddress sender = {};
		i32 bytesRead = Net_Read(g_socket, &sender, &buf, KiloBytes(64));
		if (bytesRead == 0)
		{
			printf(".");
			continue;
		}
		printf("\n");
		printf("Read %d bytes from %d.%d.%d.%d:%d\n",
			bytesRead,
			sender.ip4Bytes[0],
			sender.ip4Bytes[1],
			sender.ip4Bytes[2],
			sender.ip4Bytes[3],
			sender.port
		);
	}
}

extern "C" void ZT_MonitorStart()
{
	printf("ZT Monitor start\n");
	Net_Init();
	g_port = ZE_MONITOR_PORT;
	g_socket = Net_OpenSocket(g_port, &g_port);
	if (g_socket == -1)
	{
		printf("Error opening port %d\n", g_port);
		return;
	}
	printf("ZT Monitor listening on port %d", g_port);
	ZMonitor_Loop();
}

#endif // ZT_MONITOR_CPP
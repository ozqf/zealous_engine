#ifndef ZT_MONITOR_CPP
#define ZT_MONITOR_CPP

#include "zt_monitor.h"

#include "../win_platform/ze_win_socket.h"
#include "windows.h"

// This is all sorts of icky but it is so the monitor can
// examine app packets...

#include "../app_base/shared/packet.h"
#include "../app_base/shared/commands.h"

internal i32 g_socket;
internal u16 g_port;

internal u8 g_stagingBuf[KiloBytes(64)];

static void ZMonitor_PrintCmdHeader(Command* h)
{
	printf("CMD type %d, size %d, sentinel 0x%X\n", h->type, h->size, h->sentinel);
}

static void ZMonitor_ExaminePacket(u8* buf, i32 numBytes, ZNetAddress sender)
{
	PacketDescriptor p;
	Packet_InitDescriptor(&p, buf, numBytes, sender);
	printf("Id %d. seq %d, flags %d, time %.3f, reliable %dB, unreliable %dB\n",
		p.header.id,
		p.header.packetSequence,
		p.header.flags,
		p.header.transmissionTime,
		p.header.numReliableBytes,
		p.header.numUnreliableBytes
	);
	printf("Deserialise check: 0x%X\n", p.deserialiseCheck);

	u8* read = p.ptr + p.unreliableOffset;
	u8* end = read + p.header.numUnreliableBytes;
	// read unreliable header:
	i32 tick = COM_ReadI32(&read);

	while (read < end)
	{
		Command* h = (Command*)read;
		i32 check = Cmd_Validate(h);
		if (check != 0)
		{
			printf("Validate err %d\n", check);
			ZMonitor_PrintCmdHeader(h);
			break;
		}
		read += h->size;
		switch (h->type)
		{
			case CMD_TYPE_PING:
			{
				CmdPing* ping = (CmdPing*)h;
				printf("Ping seq %d\n", ping->pingSequence);
			} break;
			case 0:
			{
				
			} break;
			default:
			{
				printf("Cmd type %d\n", h->type);
			} break;
		}
	}
}

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
		ZMonitor_ExaminePacket(buf, bytesRead, sender);
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
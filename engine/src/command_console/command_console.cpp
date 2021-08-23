#include "../../internal_headers/zengine_internal.h"

internal ZEBuffer g_queue;
internal ZEBuffer g_execute;

ze_external void ZCmdConsole_QueueCommand(char* cmd)
{
	i32 len = ZStr_Len(cmd);
	i32 size = sizeof(i32) + sizeof(i32) + len;
	if (len >= g_queue.Space())
	{
		printf("No space to buffer commnd \"%s\"\n", cmd);
		return;
	
	}
	*(i32*)g_queue.cursor = ZE_SENTINEL;
	g_queue.cursor += sizeof(i32);
	*(i32*)g_queue.cursor = len;
	g_queue.cursor += sizeof(i32);
	g_queue.cursor += ZE_COPY(cmd, g_queue.cursor, len);
}

ze_external void ZCmdConsole_RegisterCommand(char* name, char* description, void* functionPtr)
{
	
}

ze_external void ZCmdConsole_Execute()
{
	if (g_queue.Written() == 0)
	{
		return;
	}
	
	g_execute.Clear(NO);
	Buf_CopyAll(&g_queue, &g_execute);
	g_queue.Clear(NO);
	printf("Execute %d text command bytes\n", g_execute.Written());

	i8* read = g_execute.start;
	i8* end = g_execute.cursor;
	while (read < end)
	{
		i32 sentinel = ZE_ReadI32(&read);
		if (sentinel != ZE_SENTINEL)
		{
			printf("Sync failure in text command buffer\n");
		}
		i32 len = ZE_ReadI32(&read);
		char* txt = (char*)read;
		read += len;
		printf("Exec %s\n", txt);
	}
}

ze_external i32 ZCmdConsole_Init()
{
	g_queue = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	g_execute = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	return ZE_ERROR_NONE;
}

ze_external i32 ZCmdConsole_Init_b()
{
	return ZE_ERROR_NONE;
}

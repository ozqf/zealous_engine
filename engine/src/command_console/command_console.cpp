#include "../../internal_headers/zengine_internal.h"

struct ZConsoleCmd
{
	char* name;
	char* description;
	ZCommand_Callback functionPtr;
};

internal ZEBuffer g_queue;
internal ZEBuffer g_execute;
internal ZEBuffer g_commandData;

#define MAX_CONSOLE_COMMAND_TYPES 256

internal ZEHashTable* g_commands;

internal void Exec_Help(char *fullString, char **tokens, i32 numTokens)
{
	printf("=== Command console help ===\n");
	for (i32 i = 0; i < g_commands->m_maxKeys; ++i)
	{
		ZEHashTableKey* key = &g_commands->m_keys[i];
		if (key->id == ZE_LT_INVALID_ID) { continue; }
		ZConsoleCmd* cmd = (ZConsoleCmd*)key->data.ptr;
		if (cmd == NULL) { continue; }
		printf("%s: %s\n", cmd->name, cmd->description);
	}
}

ze_external void ZCmdConsole_QueueCommand(char* cmd)
{
	if (cmd == NULL) { return; }
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

ze_external void ZCmdConsole_RegisterCommand(
	char *name, char *description, ZCommand_Callback functionPtr)
{
	ZConsoleCmd* cmd = (ZConsoleCmd*)g_commandData.cursor;
	g_commandData.cursor += sizeof(ZConsoleCmd);
	cmd->functionPtr = functionPtr;
	i32 nameLen = ZStr_Len(name);
	i32 descriptionLen = ZStr_Len(description);

	cmd->name = (char*)g_commandData.cursor;
	g_commandData.cursor += ZE_COPY(name, g_commandData.cursor, nameLen);
	cmd->description = (char *)g_commandData.cursor;
	g_commandData.cursor += ZE_COPY(description, g_commandData.cursor, descriptionLen);
	i32 id = ZE_Hash_djb2((uChar*)name);
	g_commands->InsertPointer(id, cmd);
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

		const i32 execBufSize = 256;
		const i32 maxTokens = 64;
		char execBuf[execBufSize];
		char* tokens[maxTokens];
		i32 numTokens = ZStr_Tokenise(txt, execBuf, tokens, maxTokens);
		if (numTokens == 0) { continue; }

		i32 id = ZE_Hash_djb2((uChar*)tokens[0]);
		ZConsoleCmd* cmd = (ZConsoleCmd*)g_commands->FindPointer(id);
		if (cmd == NULL)
		{
			printf("No command \"%s\" found\n", tokens[0]);
			continue;
		}
		if (cmd->functionPtr == NULL) { continue; }
		cmd->functionPtr(txt, tokens, numTokens);
	}
}

ze_external i32 ZCmdConsole_Init()
{
	g_queue = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	g_execute = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	g_commandData = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	g_commands = ZE_HashTable_Create(Platform_Alloc, MAX_CONSOLE_COMMAND_TYPES, NULL);

	ZCmdConsole_RegisterCommand("help", "List commands", Exec_Help);
	return ZE_ERROR_NONE;
}

ze_external i32 ZCmdConsole_Init_b()
{
	return ZE_ERROR_NONE;
}

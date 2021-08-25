#include "../../internal_headers/zengine_internal.h"

#define MAX_CONSOLE_COMMAND_TYPES 256

struct ZConsoleCmd
{
	char* name;
	char* description;
	i32 bExternal;
	ZCommand_Callback functionPtr;
};

// commands are queued up and executed at the start of a frame
// the queue is copied before execution, so commands queued
// during execution will run on the next frame.
// I guess it could also make it thread-safe?
internal ZEBuffer g_queue;
internal ZEBuffer g_execute;

// linear buffer storing ZConsoleCmd items and their strings.
internal ZEBuffer g_commandData;
// Hash table to lookup ZConsoleCmd items in g_commandData
internal ZEHashTable* g_commands;

internal ZEBuffer g_consoleText;

internal i32 g_bTextInputOn = NO;

ZCMD_CALLBACK(Exec_Help)
{
	printf("============================\n");
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

internal void ResetConsoleText()
{
	g_consoleText.Clear(NO);
	*(g_consoleText.cursor) = '<';
	*(g_consoleText.cursor + 1) = '\0';
}

ze_external void ZCmdConsole_SetInputEnabled(i32 flag)
{
	g_bTextInputOn = flag;
	printf("Console enabled: %s\n", flag ? "yes" : "no");
}

ze_external i32 ZCmdConsole_GetInputEnabled()
{
	return g_bTextInputOn;
}

ze_external void ZCmdConsole_WriteChar(char c, i32 bShiftOn)
{
	// printf("Console write char %c code %d\n", c, c);
	if (c == 1)
	{
		ZCmdConsole_SubmitText();
		return;
	}
	// backspace
	if (c == 3)
	{
		if (g_consoleText.Written() == 0)
		{
			return;
		}
		g_consoleText.cursor -= 1;
		*(g_consoleText.cursor) = '<';
		*(g_consoleText.cursor + 1) = '\0';
		printf("Console: %s\n", g_consoleText.start);
		return;
	}
	// A - Z is 65 - 90
	if (c >= 65 && c <= 90 && !bShiftOn)
	{
		c += 32;
	}

	*g_consoleText.cursor = c;
	g_consoleText.cursor += 1;
	*(g_consoleText.cursor) = '<';
	*(g_consoleText.cursor + 1) = '\0';
	printf("Console: %s\n", g_consoleText.start);
}

ze_external void ZCmdConsole_SubmitText()
{
	*(g_consoleText.cursor) = '\0';
	ZCmdConsole_QueueCommand((char*)g_consoleText.start);
	ResetConsoleText();
}

ze_external zErrorCode ZCmdConsole_QueueCommand(char* cmd)
{
	if (cmd == NULL) { return ZE_ERROR_NULL_ARGUMENT; }
	i32 len = ZStr_Len(cmd);
	i32 size = sizeof(i32) + sizeof(i32) + len;
	// is this text just a terminator?
	if (len <= 1)
	{
		return ZE_ERROR_BAD_ARGUMENT;
	}
	if (len >= g_queue.Space())
	{
		printf("No space to buffer commnd \"%s\"\n", cmd);
		return ZE_ERROR_NO_SPACE;
	}
	*(i32*)g_queue.cursor = ZE_SENTINEL;
	g_queue.cursor += sizeof(i32);
	*(i32*)g_queue.cursor = len;
	g_queue.cursor += sizeof(i32);
	g_queue.cursor += ZE_COPY(cmd, g_queue.cursor, len);
	return ZE_ERROR_NONE;
}

internal zErrorCode ZCmdConsole_RegisterCommand(
	char *name, char *description, i32 bExternal, ZCommand_Callback functionPtr)
{
	if (name == NULL || description == NULL || functionPtr == NULL)
	{ return ZE_ERROR_NULL_ARGUMENT; }
	
	i32 newId = ZE_Hash_djb2((uChar*)name);

	ZConsoleCmd *existingCmd = (ZConsoleCmd *)g_commands->FindPointer(newId);
	if (existingCmd != NULL)
	{
		printf("!Command \"%s\" is already registered\n", name);
		return ZE_ERROR_IDENTIFIER_ALREADY_TAKEN;
	}
	
	i32 nameLen = ZStr_Len(name);
	if (nameLen == 0) { return ZE_ERROR_BAD_ARGUMENT; }
	i32 descriptionLen = ZStr_Len(description);
	if (descriptionLen == 0) { return ZE_ERROR_BAD_ARGUMENT; }
	
	i32 totalBytes = sizeof(ZConsoleCmd) + nameLen + descriptionLen;
	if (totalBytes > g_commandData.Space()) { return ZE_ERROR_NO_SPACE; }
	
	ZConsoleCmd* cmd = (ZConsoleCmd*)g_commandData.cursor;
	g_commandData.cursor += sizeof(ZConsoleCmd);
	cmd->functionPtr = functionPtr;

	cmd->name = (char*)g_commandData.cursor;
	g_commandData.cursor += ZE_COPY(name, g_commandData.cursor, nameLen);
	cmd->description = (char *)g_commandData.cursor;
	g_commandData.cursor += ZE_COPY(description, g_commandData.cursor, descriptionLen);
	g_commands->InsertPointer(newId, cmd);
	return ZE_ERROR_NONE;
}

// Engine components should register commands here
ze_external zErrorCode ZCmdConsole_RegisterInternalCommand(
	char *name, char *description, ZCommand_Callback functionPtr)
{
	return ZCmdConsole_RegisterCommand(name, description, NO, functionPtr);
}

// game DLL registers through here
internal zErrorCode ZCmdConsole_RegisterExternalCommand(
	char *name, char *description, ZCommand_Callback functionPtr)
{
	return ZCmdConsole_RegisterCommand(name, description, YES, functionPtr);
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
	// printf("Execute %d text command bytes\n", g_execute.Written());

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
		// printf("Exec %s\n", txt);

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

ze_external ZTextCommand ZCmdConsole_RegisterFunctions()
{
	ZTextCommand r = {};
	r.QueueCommand = ZCmdConsole_QueueCommand;
	r.RegisterCommand = ZCmdConsole_RegisterExternalCommand;
	return r;
}

ze_external i32 ZCmdConsole_Init()
{
	g_queue = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	g_execute = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	g_consoleText = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	ResetConsoleText();
	g_commandData = Buf_FromMalloc(Platform_Alloc, KiloBytes(32));
	g_commands = ZE_HashTable_Create(Platform_Alloc, MAX_CONSOLE_COMMAND_TYPES, NULL);

	ZCmdConsole_RegisterInternalCommand("help", "List commands", Exec_Help);
	
	
	ZCmdConsole_WriteChar('h', NO);
	ZCmdConsole_WriteChar('e', NO);
	ZCmdConsole_WriteChar('l', NO);
	ZCmdConsole_WriteChar('p', NO);
	ZCmdConsole_SubmitText();
	return ZE_ERROR_NONE;
}

ze_external i32 ZCmdConsole_Init_b()
{
	return ZE_ERROR_NONE;
}

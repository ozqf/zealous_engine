#ifndef COMMAND_REGISTER_H
#define COMMAND_REGISTER_H

#include "zqf_network.h"

static NetCommandType g_cmdTypes[256];
static i32 g_nextCmdType = 0;

static NetCommandType* Net_GetCommandType(i32 typeId)
{
	for (i32 i = 0; i < g_nextCmdType; ++i)
	{
		if (g_cmdTypes[i].typeId = typeId)
		{
			return &g_cmdTypes[i];
		}
	}
	return NULL;
}

extern "C" void Net_RegisterCommand(
	i32 typeId,
	char* label,
	CmdFn_Serialise serialiseFn,
	CmdFn_Deserialise deserialiseFn,
	CmdFn_MeasureForSerialise measureForSerialiseFn
	)
{
	NetCommandType* newType = &g_cmdTypes[g_nextCmdType++];
	newType->typeId = typeId;
	newType->label = label;
	
	newType->serialiseFn = serialiseFn;
	newType->deserialiseFn = deserialiseFn;
	newType->measureForSerialiseFn = measureForSerialiseFn;
}
#if 0
extern "C" i32 Net_Serialise(NetCommand* cmd, u8* packet, i32 capacity)
{
	NetCommandType* t = Net_GetCommandType(cmd->type);
	if (t == NULL)
	{
		return NET_COMMAND_ERROR_NO_TYPE;
	}
	t->serialiseFn();
	return NET_COMMAND_ERROR_UNKNOWN;
}

extern "C" i32 Net_Deserialise(u8* source,  u8* dest, i32 destCapacity)
{
	return NET_COMMAND_ERROR_UNKNOWN;
}
#endif
#endif // COMMAND_REGISTER_H
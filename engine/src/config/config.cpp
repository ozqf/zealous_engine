#include "../../internal_headers/zengine_internal.h"

internal char* g_cmdLine;
internal char** g_argv;
internal i32 g_argc;

//////////////////////////////////////////
// Launch parameters
//////////////////////////////////////////
ze_external i32 ZCFG_FindParamIndex(const char *shortQuery, const char *longQuery, i32 extraTokens)
{
	i32 index = ZE_ERROR_BAD_INDEX;
	index = ZE_FindParamIndex((const char **)g_argv, g_argc, (const char *)shortQuery, extraTokens);
	if (index != ZE_ERROR_BAD_INDEX)
	{
		return index;
	}
	index = ZE_FindParamIndex((const char **)g_argv, g_argc, longQuery, extraTokens);
	return index;
}

ze_external char* ZCFG_GetParamByIndex(const i32 index)
{
	if (index < 0 || index >= g_argc)
	{
		return "";
	}
	return g_argv[index];
}

ze_external i32 ZCFG_FindIntParam(const char* shortQuery, const char* longQuery, i32 failResponse)
{
	i32 index = ZCFG_FindParamIndex(shortQuery, longQuery, 1);
	if (index == ZE_ERROR_BAD_INDEX)
	{
		printf("Could not find param %s/%s\n", shortQuery, longQuery);
		return failResponse;
	}
	printf("Found param %s: %s\n", longQuery, ZCFG_GetParamByIndex(index + 1));
	i32 i = ZStr_AsciToInt32(ZCFG_GetParamByIndex(index + 1));
	printf("Found Int param %s: %d\n", longQuery, i);
	return i;
}

//////////////////////////////////////////
// Config variables
//////////////////////////////////////////
/*
TODO - configuration values + save/load

*/
ZCMD_CALLBACK(Exec_SaveConfig)
{
	printf("Save cfg\n");
}

ZCMD_CALLBACK(Exec_LoadConfig)
{
	printf("Load cfg\n");
}

ZCMD_CALLBACK(Exec_Set)
{
	printf("Set var\n");
}

//////////////////////////////////////////
// Initialisation
//////////////////////////////////////////
ze_external zErrorCode ZCFG_Init(const char *cmdLine, const char **argv, const i32 argc)
{
	g_cmdLine = (char*)cmdLine;
	g_argv = (char**)argv;
	g_argc = argc;
	printf("ZCFG Init with %d args\n", argc);
	for (i32 i = 0; i < argc; ++i)
	{
		printf("%s, ", argv[i]);
	}
	printf("\n");
	return ZE_ERROR_NONE;
}

ze_external zErrorCode ZCFG_RegisterFunctions()
{
	return NO;
}

ze_external zErrorCode ZCFG_RegisterTextCommands()
{
	ZCmdConsole_RegisterInternalCommand(
		"set",
		"Set a configuration variable",
		Exec_Set);
	ZCmdConsole_RegisterInternalCommand(
		"savecfg",
		"Save a configuration file. If name is not provided as a second param a default is used.",
		Exec_SaveConfig);
	ZCmdConsole_RegisterInternalCommand(
		"loadcfg",
		"Load a configuration file. If name is not provided as a second param a default is used.",
		Exec_LoadConfig);
	return NO;
}

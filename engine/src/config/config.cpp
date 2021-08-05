#include "../../../headers/zengine.h"

internal char* g_cmdLine;
internal char** g_argv;
internal i32 g_argc;

ze_external i32 ZCFG_Init(const char *cmdLine, const char **argv, const i32 argc)
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

ze_external const char* ZCFG_GetParamByIndex(const i32 index)
{
	if (index < 0 || index >= g_argc)
	{
		return "";
	}
	return g_argv[index];
}

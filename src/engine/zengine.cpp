#include "../../headers/zengine.h"

ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc)
{
	ZCFG_Init(cmdLine, argv, argc);
	return ZE_ERROR_NONE;
}

#ifndef ZENGINE_H
#define ZENGINE_H

#include "ze_common.h"

#include "engine/ze_config.h"

// initialisation
ze_external zErrorCode ZE_InitConfig(const char* cmdLine, const char** argv, const i32 argc);

// config
ze_external i32 ZCFG_Init(const char *cmdLine, const char **argv, const i32 argc);
ze_external i32 ZCFG_FindParamIndex(const char* shortQuery, const char* longQuery, i32 extraTokens);

#endif // ZENGINE_H

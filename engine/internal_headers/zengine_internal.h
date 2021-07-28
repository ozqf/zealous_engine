#ifndef ZENGINE_INTERNAL_H
#define ZENGINE_INTERNAL_H

#include "../../headers/ze_common.h"

ze_external void Platform_PollEvents();
ze_external void Platform_Draw();

// initialisation
ze_external zErrorCode ZE_InitConfig(const char* cmdLine, const char** argv, const i32 argc);

// config
ze_external i32 ZCFG_Init(const char *cmdLine, const char **argv, const i32 argc);
ze_external i32 ZCFG_FindParamIndex(const char* shortQuery, const char* longQuery, i32 extraTokens);

ze_external i32 ZE_StartLoop();
ze_external void ZE_Shutdown();

#endif // ZENGINE_INTERNAL_H

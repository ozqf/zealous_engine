#ifndef SERVER_H
#define SERVER_H

#include "../../ze_common/ze_common.h"
#include "../shared/user.h"

extern "C" void    SV_Init();
extern "C" void    SV_Start();
extern "C" i32     SV_IsRunning();
extern "C" void    SV_Shutdown();
extern "C" UserIds SVU_CreateLocalUser();
extern "C" void    SV_Tick(ZEByteBuffer* platformCommands, timeFloat deltaTime);
extern "C" u8      SV_ParseCommandString(const char* str, const char** tokens, const i32 numTokens);

// For debugging ONLY
extern "C" void    SV_Debug_GetSimInstance(void** ptr);

#endif // SERVER_H
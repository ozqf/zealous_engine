#ifndef ZE_WIN_SOCKET_H
#define ZE_WIN_SOCKET_H

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_net_types.h"

///////////////////////////////////////
// Functions
///////////////////////////////////////

i32 Net_Init();
i32 Net_OpenSocket(u16 port, u16* portOpened);
i32 Net_CloseSocket(i32 socketIndex);
i32 Net_Shutdown();

extern "C"
i32 Net_SendTo(i32 transmittingSocketIndex, ZNetAddress* address, u16 port, u8* data, i32 dataSize);
i32 Net_Read(i32 socketIndex, ZNetAddress* sender, u8** buf, i32 bufSize);

void Net_RunLoopbackTest();

#endif // ZE_WIN_SOCKET_H

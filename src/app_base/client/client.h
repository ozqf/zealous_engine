#ifndef CLIENT_H
#define CLIENT_H

/**
 * client public interface
 */
#include "../../ze_common/ze_common_full.h"
#include "../shared/user.h"

#define CL_MAX_SENT_INPUT_COMMANDS 60

#define CL_DEBUG_FLAG_DRAW_LOCAL_SERVER (1 << 0)
#define CL_DEBUG_FLAG_NO_ENEMY_TICK (1 << 1)
#define CL_DEBUG_FLAG_NO_PLAYER_SMOOTHING (1 << 2)
#define CL_DEBUG_FLAG_DRAW_REAL_LOCAL_POSITION (1 << 3)
#define CL_DEBUG_FLAG_DEBUG_CAMERA (1 << 4)

#define CL_DEBUG_FLAG_VERBOSE_FRAME (1 << 31)

#define CLIENT_STATE_NONE 0
#define CLIENT_STATE_REQUESTING 1
#define CLIENT_STATE_HANDSHAKE 2
#define CLIENT_STATE_SYNC 3
#define CLIENT_STATE_PLAY 4

extern "C" void	CL_Init();
extern "C" void	CL_Start(ZNetAddress serverAddress, i32 updSocketId);
extern "C" void	CL_Shutdown();
extern "C" i32	CL_IsRunning();
extern "C" void	CL_CopyCameraTransform(Transform* target);
extern "C" void	CL_Tick(ZEByteBuffer* sysEvents, timeFloat deltaTime, i64 platformFrame);
extern "C" void	CL_WriteDrawFrame(ZRViewFrame* frame);
extern "C" void	CL_SetLocalUser(UserIds ids);
extern "C" void	CL_WriteDebugString(CharBuffer* str);
extern "C" u8  	CL_ParseCommandString(const char* str, const char** tokens, const i32 numTokens);

#endif // CLIENT_H
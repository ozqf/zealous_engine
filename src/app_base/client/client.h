#ifndef CLIENT_H
#define CLIENT_H

#include "../user.h"
#include "../../ze_common/ze_common.h"
#include "../../ze_common/ze_char_buffer.h"
#include "../app.h"
#include "../commands.h"
#include "../stream.h"
//#include "../../renderer_interface.h"

#define CL_MAX_SENT_INPUT_COMMANDS 60

extern "C" void CL_Init(ZNetAddress serverAddress);
extern "C" void CL_Shutdown();
extern "C" i32 CL_IsRunning();
extern "C" void CL_CopyCameraTransform(Transform* target);
extern "C" void CL_Tick(
			ZEByteBuffer* sysEvents,
			timeFloat deltaTime,
			i64 platformFrame);
extern "C" void CL_WriteDrawFrame(ZEByteBuffer* list, ZEByteBuffer* data);
/*
void    CL_PopulateRenderScene(
			Transform* cam,
			RenderScene* scene,
			i32 maxObjects,
			i32 texIndex,
			f32 interpolateTime);
void    CL_GetRenderCommands(
			RenderCommand** cmds,
			i32* numCommands,
			i32 texIndex,
			f32 interpolateTime);
*/
extern "C" void    CL_SetLocalUser(UserIds ids);
extern "C" void    CL_WriteDebugString(CharBuffer* str);
extern "C" u8      CL_ParseCommandString(
			char* str,
			char** tokens,
			i32 numTokens);

#endif // CLIENT_H
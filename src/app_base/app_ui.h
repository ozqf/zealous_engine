#ifndef APP_UI_H
#define APP_UI_H

//#include "../ui/zui.h"
#include "../ze_common/ze_common.h"
#include "../zqf_renderer.h"
#include "../sys_events.h"

extern "C" void AppUI_Init();
extern "C" void AppUI_Update(Vec2 mouseScreenPos);
extern "C" i32 AppUI_IsActive();
extern "C" ErrorCode AppUI_WriteFrame(ZRViewFrame* frame);
extern "C" void AppUI_ProcessInput(SysInputEvent ev);
extern "C" void AppUI_OpenRoot();

#endif // APP_UI_H
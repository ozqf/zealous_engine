#ifndef APP_UI_H
#define APP_UI_H

#define APPUI_MENU_TOGGLE_KEY Z_INPUT_CODE_ESCAPE

#define APPUI_INPUT_UNHANDLED 0
#define APPUI_INPUT_HANDLED 1
#define APPUI_INPUT_TOGGLED_ON 2
#define APPUI_INPUT_TOGGLED_OFF 3

//#include "../ui/zui.h"
#include "../ze_common/ze_common.h"
#include "../zqf_renderer.h"
#include "../sys_events.h"
#include "../ze_module_interfaces.h"

extern "C" void AppUI_Init(ze_platform_export platform);
extern "C" void AppUI_Update();
extern "C" i32 AppUI_IsActive();
extern "C" ErrorCode AppUI_WriteFrame(ZRViewFrame* frame);
// return 0 == unhandled, 1 == handled, 2 == menu toggled
extern "C" i32 AppUI_ProcessInput(SysInputEvent ev);

#endif // APP_UI_H
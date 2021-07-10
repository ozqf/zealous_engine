/*
Simple 2D game implementation
*/
#include "../../headers/common/ze_common_full.h"
#include "../../headers/zqf_renderer.h"
#include "../../headers/ze_module_interfaces.h"
#include "../../headers/sys_events.h"

extern "C" void G2d_Init(ze_platform_export platform);
extern "C" void G2d_Tick(f32 delta);
extern "C" void G2d_InputEvent(SysInputEvent *input);
extern "C" void G2d_Draw(ZRViewFrame *frame);
extern "C" void G2d_Shutdown();

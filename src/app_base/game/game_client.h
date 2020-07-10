
#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../../sys_events.h"

// Input
extern "C" void CL_Init();
extern "C" void CL_ReadInputEvent(SysInputEvent* ev, frameInt frameNumber);
extern "C" void CLG_Start();
extern "C" void CL_Stop();

// If local client - write input
extern "C" void CL_PreTick(timeFloat delta);
// If remote client - send state
extern "C" void CL_PostTick(timeFloat delta);

extern "C" Transform CL_GetDebugCamera();

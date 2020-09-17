
#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../../sys_events.h"
#include "../game_draw/game_draw.h"

// Session
extern "C" void CL_Init(ZE_FatalErrorFunction fatalFunc);
extern "C" void CLG_Start();
extern "C" void CL_Stop();

extern "C" void CL_ReadInputEvent(SysInputEvent* ev, frameInt frameNumber);
extern "C" void CL_RegisterLocalPlayer(SimScene* sim, SimPlayer plyr);

// If local client - write input
extern "C" void CL_PreTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta);
// If remote client - send local avatar state
extern "C" void CL_PostTick(SimScene* sim, ZEDoubleByteBuffer* buf, timeFloat delta);

extern "C" Transform CL_GetCamera(SimScene* sim);
extern "C" ClientView CL_GetClientView(SimScene* sim);

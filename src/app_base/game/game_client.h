
#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../../sys_events.h"
#include "../game_draw/game_draw.h"

// Session
extern "C" void CL_Init(ZE_FatalErrorFunction fatalFunc, ZRAssetDB* db);
extern "C" void CL_Start(SimScene* sim);
extern "C" void CL_Stop();
extern "C" void CL_Save(SimScene* sim, SimSaveFileInfo* saveInfo, i32 file, ZEFileIO files);
extern "C" void CL_Resume(SimScene* sim, SimSaveFileInfo* saveInfo, ZEBuffer* saveData);

extern "C" void CL_ReadInputEvent(SysInputEvent* ev, frameInt frameNumber);
extern "C" void CL_RegisterLocalPlayer(SimScene* sim, i32 playerId);

// If local client - write input
extern "C" void CL_PreTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta);
// If remote client - send local avatar state
extern "C" void CL_PostTick(SimScene* sim, ZEDoubleBuffer* buf, timeFloat delta);
extern "C" void CL_WriteDrawFrame(SimScene* sim, ZRViewFrame* frame);

extern "C" Transform CL_GetCamera(SimScene* sim);
extern "C" ClientView CL_GetClientView(SimScene* sim);
extern "C" void CL_ClearActionInputs();
extern "C" void CL_ClearDebugFlags();
extern "C" void CL_ToggleDrawFlag(i32 flag);

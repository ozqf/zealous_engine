
#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../../sys_events.h"

extern "C" void GI_InitInputs(InputActionSet* actions);
extern "C" void GI_ReadInputEvent(InputActionSet* actions, SysInputEvent* ev, frameInt frameNumber);
extern "C" void GI_UpdateActorInput(InputActionSet* actions, SimActorInput* input);

#ifndef ZE_PHYSICS2D_H
#define ZE_PHYSICS2D_H

#include "../headers/zengine.h"

// edit
ze_external zeHandle ZP_AddStaticVolume(Vec2 pos, Vec2 size);
ze_external zeHandle ZP_AddDynamicVolume(Vec2 pos, Vec2 size);

// query
ze_external Vec2 ZP_GetBodyPosition(zeHandle bodyId);

// lifetime
ze_external void ZPhysicsInit(ZEngine engine);
ze_external void ZPhysicsTick(f32 delta);

#endif ZE_PHYSICS2D_H

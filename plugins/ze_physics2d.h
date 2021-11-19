#ifndef ZE_PHYSICS2D_H
#define ZE_PHYSICS2D_H

#include "../headers/zengine.h"

struct Transform2d
{
    Vec2 pos;
    f32 radians;
};


// construct
ze_external zeHandle ZP_AddStaticVolume(Vec2 pos, Vec2 size);
ze_external zeHandle ZP_AddDynamicVolume(Vec2 pos, Vec2 size);

// affect
ze_external void ZP_ApplyForce(zeHandle bodyId, Vec2 force);

// query
ze_external Transform2d ZP_GetBodyPosition(zeHandle bodyId);

// lifetime
ze_external void ZPhysicsInit(ZEngine engine);
ze_external void ZPhysicsTick(f32 delta);

#endif ZE_PHYSICS2D_H

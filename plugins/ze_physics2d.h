#ifndef ZE_PHYSICS2D_H
#define ZE_PHYSICS2D_H

#include "../headers/zengine.h"

struct Transform2d
{
    Vec2 pos;
    f32 radians;
};

struct BodyState
{
    Transform2d t;
    Vec2 velocity;
};

struct ZPShapeDef
{
    Vec2 pos;
    Vec2 size;
    f32 resitition;
    f32 friction;
};

// construct
ze_external zeHandle ZP_AddStaticVolume(Vec2 pos, Vec2 size);
ze_external zeHandle ZP_AddDynamicVolume(ZPShapeDef def);

// affect
ze_external void ZP_ApplyForce(zeHandle bodyId, Vec2 force);
ze_external void ZP_SetBodyPosition(zeHandle bodyId, Vec2 pos);
ze_external void ZP_SetLinearVelocity(zeHandle bodyId, Vec2 v);

// query
ze_external Transform2d ZP_GetBodyPosition(zeHandle bodyId);
ze_external BodyState ZP_GetBodyState(zeHandle bodyId);

// lifetime
ze_external void ZPhysicsInit(ZEngine engine);
ze_external void ZPhysicsTick(f32 delta);

#endif ZE_PHYSICS2D_H

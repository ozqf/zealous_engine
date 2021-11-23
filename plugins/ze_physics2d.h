#ifndef ZE_PHYSICS2D_H
#define ZE_PHYSICS2D_H

#define ZP_SHAPE_TYPE_BOX 0
#define ZP_SHAPE_TYPE_CIRCLE 1

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
    // if shapetype == circle, only x component of size is used.
    i32 shapeType;
    f32 resitition;
    f32 friction;
};

struct ZPBodyDef
{
    i32 bIsStatic;
    i32 bLockRotation;
    ZPShapeDef shape;
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
ze_external void ZP_Raycast(Vec2 from, Vec2 to);

// lifetime
ze_external void ZPhysicsInit(ZEngine engine);
ze_external void ZPhysicsTick(f32 delta);

#endif ZE_PHYSICS2D_H

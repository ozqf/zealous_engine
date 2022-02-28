#ifndef ZE_PHYSICS2D_H
#define ZE_PHYSICS2D_H
/*
2D physics public interface
*/
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
    f32 angularVelocity;
    i32 externalId;
};

struct ZPShapeDef
{
    Vec2 pos;
    Vec2 radius;
    // if shapetype == circle, only x component of size is used.
    i32 shapeType;
};

struct ZPBodyDef
{
    i32 externalId;
    i32 bIsStatic;
    i32 bLockRotation;
    f32 resitition;
    f32 friction;
    ZPShapeDef shape;
};

struct ZPRaycastResult
{
    Vec2 pos;
    Vec2 normal;
    f32 fraction;
    i32 volumeId;
    i32 externalId;
};

// construct
// ze_external zeHandle ZP_AddDynamicVolume(ZPShapeDef def);
ze_external zeHandle ZP_AddStaticVolume(Vec2 pos, Vec2 size);
ze_external zeHandle ZP_AddBody(ZPBodyDef def);
ze_external zErrorCode ZP_RemoveBody(zeHandle bodyId);

// affect
ze_external void ZP_ApplyForce(zeHandle bodyId, Vec2 force);
ze_external void ZP_SetBodyPosition(zeHandle bodyId, Vec2 pos);
ze_external void ZP_SetLinearVelocity(zeHandle bodyId, Vec2 v);
ze_external void ZP_SetBodyState(zeHandle bodyId, BodyState state);

// query
ze_external Transform2d ZP_GetBodyPosition(zeHandle bodyId);
ze_external BodyState ZP_GetBodyState(zeHandle bodyId);
// returns number of results.
ze_external i32 ZP_Raycast(
    Vec2 from, Vec2 to, ZPRaycastResult* results, i32 maxResults);

// lifetime
ze_external void ZPhysicsInit(ZEngine engine);
ze_external void ZPhysicsTick(f32 delta);

#endif ZE_PHYSICS2D_H

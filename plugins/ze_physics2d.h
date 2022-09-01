#ifndef ZE_PHYSICS2D_H
#define ZE_PHYSICS2D_H
/*
2D physics public interface
Any 2D physics implementations should use this interface
*/
#define ZP_SHAPE_TYPE_BOX 0
#define ZP_SHAPE_TYPE_CIRCLE 1

#define ZP_MASK_ALL 65535

#define ZP_EMPTY_ID 0

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
    u16 categoryBits;
    u16 maskBits;
    ZPShapeDef shape;
};

struct ZAABBResult
{
    i32 volumeId;
    i32 externalId;
};

struct ZPRaycastResult
{
    Vec2 pos;
    Vec2 normal;
    f32 fraction;
    i32 volumeId;
    i32 externalId;
    u16 categoryBits;
};

// construct
// ze_external zeHandle ZP_AddDynamicVolume(ZPShapeDef def);
ze_external zeHandle ZP_AddStaticVolume(
    Vec2 pos, Vec2 size, u16 categoryBits, u16 maskBits);
ze_external zeHandle ZP_AddBody(ZPBodyDef def);
ze_external zErrorCode ZP_RemoveBody(zeHandle bodyId);

// affect
ze_external void ZP_ApplyForce(zeHandle bodyId, Vec2 force);
ze_external void ZP_ApplyForceAtPoint(zeHandle bodyId, Vec2 force, Vec2 point);
ze_external void ZP_SetBodyPosition(zeHandle bodyId, Vec2 pos);
ze_external void ZP_SetLinearVelocity(zeHandle bodyId, Vec2 v);
ze_external void ZP_SetBodyState(zeHandle bodyId, BodyState state);
ze_external void ZP_SetBodyMaskBit(zeHandle bodyId, u16 maskBit, i32 bOn);

// query
ze_external ZPShapeDef ZP_GetBodyShape(zeHandle bodyId);
ze_external Transform2d ZP_GetBodyPosition(zeHandle bodyId);
ze_external BodyState ZP_GetBodyState(zeHandle bodyId);
// returns number of results
ze_external i32 ZP_Raycast(
    Vec2 from, Vec2 to, ZPRaycastResult* results, i32 maxResults, u16 mask);
// returns number of results.
ze_external i32 ZP_AABBCast(
	Vec2 min, Vec2 max, ZAABBResult* results, i32 maxResults, u16 mask);
ze_external i32 ZP_GroundTest(zeHandle physicsBody, u16 mask);

// lifetime
ze_external void ZPhysicsInit(ZEngine engine);
ze_external void ZPhysicsTick(f32 delta);

#endif ZE_PHYSICS2D_H

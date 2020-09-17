#pragma once

#include "bullet_internal_types.h"

////////////////////////////////////////////////////////////
/*
COPYED FROM ORIGINAL BULLET SOURCE
- Missing in header files for lib
*/
	enum CollisionFlags
	{
		CF_STATIC_OBJECT= 1,
		CF_KINEMATIC_OBJECT= 2,
		CF_NO_CONTACT_RESPONSE = 4,
		CF_CUSTOM_MATERIAL_CALLBACK = 8,//this allows per-triangle material (friction/restitution)
		CF_CHARACTER_OBJECT = 16,
		CF_DISABLE_VISUALIZE_OBJECT = 32, //disable debug drawing
		CF_DISABLE_SPU_COLLISION_PROCESSING = 64,//disable parallel/SPU processing
		CF_HAS_CONTACT_STIFFNESS_DAMPING = 128,
		CF_HAS_CUSTOM_DEBUG_RENDERING_COLOR = 256,
		CF_HAS_FRICTION_ANCHOR = 512,
		CF_HAS_COLLISION_SOUND_TRIGGER = 1024
	};

	enum	CollisionObjectTypes
	{
		CO_COLLISION_OBJECT =1,
		CO_RIGID_BODY=2,
		///CO_GHOST_OBJECT keeps track of all objects overlapping its AABB and that pass its collision filter
		///It is useful for collision sensors, explosion objects, character controller etc.
		CO_GHOST_OBJECT=4,
		CO_SOFT_BODY=8,
		CO_HF_FLUID=16,
		CO_USER_TYPE=32,
		CO_FEATHERSTONE_LINK=64
	};

	enum AnisotropicFrictionFlags
	{
		CF_ANISOTROPIC_FRICTION_DISABLED=0,
		CF_ANISOTROPIC_FRICTION = 1,
		CF_ANISOTROPIC_ROLLING_FRICTION = 2
	};
////////////////////////////////////////////////////////////

enum ZCommandType
{
	Null = 0,
	Teleport = 1,
	Create = 2,
	Remove = 3,
	SetVelocity = 4,
	SetState = 5,
	Raycast = 6
};

internal PhysBodyHandle* Phys_GetFreeBodyHandle(PhysBodyList* list);
internal PhysBodyHandle* Phys_GetHandleById(PhysBodyList* list, i32 queryId);
internal void Phys_FreeHandle(ZBulletWorld* world, PhysBodyHandle* handle);
internal void Phys_RecycleHandle(ZBulletWorld* world, PhysBodyHandle* handle);
internal i32 PhysCmd_RayTest(ZBulletWorld* world, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 y2);
i32 Phys_QuickRaycast(ZBulletWorld *world, PhysCmd_Raycast* cmd, PhysRayHit* hits, i32 maxHits);
i32 Phys_ExecRaycast(ZBulletWorld *world, PhysCmd_Raycast* cmd, u8* buf, u32 bufferCapacity);
//PhysBodyHandle* Phys_CreateBulletSphere(ZBulletWorld* world, ZSphereDef def);
internal PhysBodyHandle* Phys_CreateBulletBox(ZBulletWorld* world, ZShapeDef* def, ZCollisionBoxData* box);
//PhysBodyHandle* Phys_CreateBulletInfinitePlane(ZBulletWorld* world, ZShapeDef def);
internal void Phys_PreSolveCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep);
internal void Phys_PostSolveCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep);
internal void Phys_ClearOverlapPairs(ZBulletWorld* world);
internal void Phys_WriteDebugOutput(ZBulletWorld* world);
internal void Phys_LockCommandBuffer(ZEByteBuffer* buffer);
internal void Phys_ReadCommands(ZBulletWorld* world, ZEByteBuffer* output);
internal void Phys_StepWorld(ZBulletWorld* world, f32 deltaTime);

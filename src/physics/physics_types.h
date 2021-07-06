#pragma once

#include "../../headers/common/ze_common.h"

typedef void (*PhysErrorHandler)(char* message);

//////////////////////////////////////////////////////////////////
// Collision Shapes
//////////////////////////////////////////////////////////////////
#define ZCOLLIDER_TYPE_CUBOID 0
#define ZCOLLIDER_TYPE_SPHERE 1
#define ZCOLLIDER_TYPE_CAPSULE 2
#define ZCOLLIDER_TYPE_MESH 3

enum ZShapeType
{
	box = 0,
	sphere = 1,
	capsule = 2,
	mesh = 3
};
//#define ZCOLLIDER_TYPE_INFINITE_PLANE 99
// TOOD: Kinematic doesn't do anything...?
#define ZCOLLIDER_FLAG_STATIC (1 << 0)
#define ZCOLLIDER_FLAG_KINEMATIC (1 << 1)
#define ZCOLLIDER_FLAG_NO_ROTATION (1 << 2)
#define ZCOLLIDER_FLAG_GROUNDCHECK (1 << 3)
#define ZCOLLIDER_FLAG_NO_COLLISION_RESPONSE (1 << 4)
// Report overlap begin/end events
#define ZCOLLIDER_FLAG_INTERESTING (1 << 5)
// Report overlaps every frame
#define ZCOLLIDER_FLAG_VERY_INTERESTING (1 << 6)

// This struct is the header of an internal data structure
// for passing back to a caller
struct WorldHandle
{
	i32 id;
};

#if 1

struct ZCollisionBoxData
{
	f32 halfSize[3];
};

struct ZCollisionSphereData
{
	f32 radius;
};
#if 0
struct ZCollisionCapsuleData
{
	f32 halfWidth;
	f32 halfHeight;
};

struct ZCollisionMeshData
{
	f32 halfWidth;
	f32 halfHeight;
};
#endif
union ZShapeData_U
{
	ZCollisionBoxData box;
	ZCollisionSphereData sphere;
	//ZCollisionCapsuleData capsule;
	//ZCollisionMeshData mesh;
};

struct ZShapeDef
{
	ZShapeType shapeType;
	// externally set value for user grouping/selection only
	i32 tag;
	
	u32 flags;
	i32 handleId;
	u16 group;
	u16 mask;
	f32 restitution;
	f32 pos[3];
	ZShapeData_U data;

	void SetAsBox(
		Vec3 position,
		Vec3 halfSize,
    	u32 newFlags,
    	u16 newGroup,
    	u16 newMask,
		i32 newTag
	)
	{
		*this = {};
    	shapeType = box;
    	flags = newFlags;
    	group = newGroup;
    	mask = newMask;
		tag = newTag;
    	pos[0] = position.x;
    	pos[1] = position.y;
    	pos[2] = position.z;
    	data.box.halfSize[0] = halfSize.x;
    	data.box.halfSize[1] = halfSize.y;
    	data.box.halfSize[2] = halfSize.z;
	}

	void SetScale(f32 halfSizeX, f32 halfSizeY, f32 halfSizeZ)
	{
		switch (this->shapeType)
		{
			case ZCOLLIDER_TYPE_CUBOID:
			{
				this->data.box.halfSize[0] = halfSizeX;
				this->data.box.halfSize[1] = halfSizeY;
				this->data.box.halfSize[2] = halfSizeZ;
			} break;
			default:
			{
				ILLEGAL_CODE_PATH;
			} break;
		}
	}
};

#endif

//////////////////////////////////////////////////////////////////
// Collision query data
//////////////////////////////////////////////////////////////////
#if 0
struct ZRayHit
{
	Vec3 pos;
	Vec3 normal;
	Vec3 penetration;
	ZCollider* victim;
};

struct ZRayHitscan
{
	int searchId;
	ZRayHit hits[100];
};

struct ZVolumeSearch
{
	int searchId;
	ZCollider* shapes[100];
};

struct ZTouchPair
{
	ZCollider* a;
	ZCollider* b;
};
#endif

//////////////////////////////////////////////////////////////////
// Commands -> incoming instructions to the physics engine
//////////////////////////////////////////////////////////////////
#if 1
struct PhysCmd_Teleport
{
	// u8 type
	i32 shapeId;
	f32 pos[3];
};

struct PhysCmd_VelocityChange
{
	// u8 type
	i32 shapeId;
	i32 mode = 0;
	f32 vel[3];
};

struct PhysCmd_Raycast
{
	i32 id;
	f32 start[3];
	f32 end[3];
	u16 group;
	u16 mask;
};

struct PhysCmd_State
{
	i32 shapeId;
	f32 pos[3];
	f32 rot[3];
	f32 vel[3];
};

#endif

//////////////////////////////////////////////////////////////////
// Events -> out going data from physics engine step
//////////////////////////////////////////////////////////////////

enum PhysEventType
{
	None = 0,
	TransformUpdate = 1,
	RaycastResult = 2,
	RaycastDebug = 3,
	OverlapStarted = 4,
	OverlapEnded = 5,
	OverlapInProgress = 6
};

//////////////////////////////////////////////////////////////////
// Header for all items in event or command buffers
//////////////////////////////////////////////////////////////////

struct PhysEv_Header
{
	PhysEventType type;
	i32 size;
	i32 sentinel;
};

#define PHYS_HEADER_SENTINEL 0xC0FFEE

// MUST be called when setting up any event
extern "C"
inline void PhysEv_SetHeader(
    PhysEv_Header* header, PhysEventType type, i32 size)
{
    header->type = type;
    header->size = size;
    header->sentinel = PHYS_HEADER_SENTINEL;
}

extern "C"
inline ErrorCode Phys_ValidateEvent(PhysEv_Header* header)
{
	if (header == NULL) { return ZE_ERROR_BAD_ARGUMENT; }
	if (header->sentinel != PHYS_HEADER_SENTINEL)
	{ return ZE_ERROR_DESERIALISE_FAILED; }
	if (header->type == 0) { return ZE_ERROR_UNKNOWN_COMMAND; }
	if (header->size <= 0) { return ZE_ERROR_BAD_SIZE; }
	return ZE_ERROR_NONE;
}

#define PHYS_UPDATE_NULL 0
#define PHYS_EVENT_TRANSFORM 1
#define PHYS_EVENT_RAYCAST 2

#define PHYS_EV_FLAG_GROUNDED (1 << 0)

struct PhysEV_TransformUpdate
{
	PhysEv_Header header;
	u32 ownerId;
	f32 matrix[16];
	f32 rot[3];
	f32 vel[3];
	f32 angularVel[3];
	u32 flags;
	i32 tag;
};

struct PhysRayHit
{
	f32 worldPos[3];
	f32 normal[3];
	i32 shapeId;
};

struct PhysEv_RaycastResult
{
	PhysEv_Header header;
	i32 queryId;
	f32 start[3];
	f32 end[3];
	i32 numHits;
};

struct PhysEv_RaycastDebug
{
	PhysEv_Header header;
	f32 a[3];
	f32 b[3];
	f32 colour[3];
};

struct PhysEv_CollisionItem
{
	PhysEv_Header header;
	u32 externalId;
	i32 shapeTag;
};

struct PhysEv_Collision
{
	PhysEv_Header header;
	PhysEv_CollisionItem a;
	PhysEv_CollisionItem b;
};

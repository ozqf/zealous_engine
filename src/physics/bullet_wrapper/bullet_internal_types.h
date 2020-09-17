#pragma once

#include "../physics.h"

#include "../../../lib/bullet/btBulletCollisionCommon.h"
#include "../../../lib/bullet/btBulletDynamicsCommon.h"


/**
 * Contains Pointers to a Bullet Physics Shape
 */
struct PhysBodyHandle
{
    u8 inUse;
    // internal id, matches user index on btRigidbody
    i32 id;
    // external id
    u32 externalId;
    
    ZShapeDef def;
    
    // pointers to bullet physics objects
    btCollisionShape* shape;
    btGhostObject* ghost;
    btDefaultMotionState* motionState;
    btRigidBody* rigidBody;
};

struct PhysBodyList
{
    PhysBodyHandle* items;
    i32 capacity;
};

struct PhysDebugInfo
{
    u32 stepCount = 0;
    u32 preSolves = 0;
    u32 postSolves = 0;

    u32 numManifolds = 0;
    u32 inputPeakBytes;
    u32 outputPeakBytes;
};

// Owner Ids must be recorded here, due to read/write order of commands and events
// (handle may be released before pair is checked...)
struct PhysOverlapItem
{
    i32 internalId;
    u32 externalId;
    i32 shapeTag;
};

struct PhysOverlapPair
{
    i32 isActive;
    u32 startFrame;
    u32 latestFrame;
    i32 reportLevel;
	
    PhysOverlapItem a;
    PhysOverlapItem b;
};

struct PhysInfinitePlane
{
    
};

#define HANDLE2WORLD ZBulletWorld* world = (ZBulletWorld*)handle;

struct ZBulletWorld
{
    WorldHandle header;
    // Identifies the world
    i32 index;
    char* label;
    // Wrapper components
    ZEByteBuffer input;
    ZEByteBuffer output;
    PhysErrorHandler errorHandler;

    PhysBodyList bodies;
    i32 maxBodies;
    PhysDebugInfo debug;

    PhysOverlapPair* overlapPairs;
    i32 maxOverlaps;
    i32 numOverlaps;
    i32 nextQueryId;
    i32 verbose;

    // bullet physics components
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;

    //btDiscreteDynamicsWorld* dynamicsWorld;
    btDynamicsWorld* dynamicsWorld;
};
#if 0
struct PhysicsTestState
{
    btCollisionShape* groundShape;
    btDefaultMotionState* groundMotionState;
    btRigidBody* groundRigidBody;

    btCollisionShape* sphereShape;
    btDefaultMotionState* sphereMotionState;
    btRigidBody* sphereRigidBody;
};
#endif

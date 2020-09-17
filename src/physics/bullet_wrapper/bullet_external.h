#pragma once

#include "bullet_module.cpp"

extern "C"
void Phys_Error(char* message)
{
    printf("PHYS Error: %s\n", message);
    if (g_errorHandler)
    {
        g_errorHandler(message);
    }
    ILLEGAL_CODE_PATH
}

/////////////////////////////////////////////////////////////
// ISSUE COMMAND
/////////////////////////////////////////////////////////////

#if 0
i32 Phys_EnqueueRaycast(PhysCmd_Raycast* ev)
{
    // Thread safe...? do you care...?
    i32 id = g_world.nextQueryId++;
    ev->id = id;
    *g_input.cursor = RayCast;
    g_input.cursor++;
    g_input.cursor += ZE_COPY_STRUCT(ev, g_input.cursor, PhysCmd_Raycast);
    return id;
}
#endif
extern "C"
i32 PhysCmd_CreateShape(WorldHandle* handle, ZShapeDef* def, u32 externalId)
{
    HANDLE2WORLD
    ZE_ASSERT(def != NULL, "No shape def provided");
    PhysBodyHandle* h = Phys_GetFreeBodyHandle(&world->bodies);
    
    ZE_ASSERT(h != NULL, "No free body handle");
    def->handleId = h->id;
	h->externalId = externalId;
    *world->input.cursor = Create;
    world->input.cursor++;
    world->input.cursor += ZE_COPY_STRUCT(def, world->input.cursor, ZShapeDef);
    if (world->verbose)
    {
        printf("PHYS Assigning shape Id %d for external %d\n", def->handleId, h->externalId);
    }
    
    return h->id;
}
#if 0
i32 PhysCmd_CreateBox(
    f32 x,
    f32 y,
    f32 z,
    f32 halfSizeX,
    f32 halfSizeY,
    f32 halfSizeZ,
    u32 flags,
    i32 tag,
    u16 group,
    u16 mask,
    u32 ownerId
    )
{
    ZShapeDef def = {};
    def.shapeType = box;
    def.flags = flags;
    def.group = group;
    def.mask = mask;
    def.tag = tag;
    def.pos[0] = x;
    def.pos[1] = y;
    def.pos[2] = z;
    def.data.box.halfSize[0] = halfSizeX;
    def.data.box.halfSize[1] = halfSizeY;
    def.data.box.halfSize[2] = halfSizeZ;
    return PhysCmd_CreateShape(&def, ownerId);
}
#endif
extern "C"
void PhysCmd_RemoveShape(WorldHandle* handle, i32 shapeId)
{
    HANDLE2WORLD
    PhysBodyHandle* h = Phys_GetHandleById(&world->bodies, shapeId);
    world->input.cursor += COM_WriteByte(Remove, world->input.cursor);
    world->input.cursor += COM_WriteI32(shapeId, world->input.cursor);
}

extern "C"
void PhysCmd_SetState(WorldHandle* handle, PhysCmd_State* state)
{
    HANDLE2WORLD
    world->input.cursor += COM_WriteByte(SetState, world->input.cursor);
    world->input.cursor += ZE_COPY_STRUCT(state, world->input.cursor, PhysCmd_State);
}

extern "C"
void PhysCmd_TeleportShape(WorldHandle* handle, i32 shapeId, f32 posX, f32 posY, f32 posZ)
{
    HANDLE2WORLD
    PhysCmd_Teleport cmd = {};
    cmd.shapeId = shapeId;
    cmd.pos[0] = posX;
    cmd.pos[1] = posY;
    cmd.pos[2] = posZ;
	
    world->input.cursor += COM_WriteByte(Teleport, world->input.cursor);
    world->input.cursor += ZE_COPY_STRUCT(&cmd, world->input.cursor, PhysCmd_Teleport);
}

extern "C"
void PhysCmd_ChangeVelocity(WorldHandle* handle, i32 shapeId, f32 velX, f32 velY, f32 velZ)
{
    HANDLE2WORLD
	PhysCmd_VelocityChange cmd = {};
	cmd.shapeId = shapeId;
	cmd.vel[0] = velX;
	cmd.vel[1] = velY;
	cmd.vel[2] = velZ;

	world->input.cursor += COM_WriteByte(SetVelocity, world->input.cursor);
	world->input.cursor += ZE_COPY_STRUCT(&cmd, world->input.cursor, PhysCmd_VelocityChange);
}

/////////////////////////////////////////////////////////////
// Querying
/////////////////////////////////////////////////////////////

// return number of hits or number of hits written if max is lower
extern "C"
i32 PhysExt_QueryRay(WorldHandle* handle, PhysCmd_Raycast* cmd, PhysRayHit* hits, i32 maxHits)
{
    HANDLE2WORLD
    return Phys_QuickRaycast(world, cmd, hits, maxHits);
}

// Return bytes written
// i32 PhysExt_QueryRay(PhysCmd_Raycast* cmd, u8* resultsBuffer, u32 bufferCapacity)
// {
//     return Phys_ExecRaycast(&g_world, cmd, resultsBuffer, bufferCapacity);
// }
extern "C"
i32 PhysExt_RayTest(WorldHandle* handle, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 y2)
{
    HANDLE2WORLD
    return PhysCmd_RayTest(world, x0, y0, z0, x1, y1, y2);
}

extern "C"
void PhysExt_GetDebugString(char** str, i32* length)
{
    *str = g_debugString;
    *length = g_debugStringSize;
}

extern "C"
i32 PhysExt_QueryHitscan()
{
    return 0;
}

extern "C"
i32 PhysExt_QueryVolume()
{
    return 0;
}

extern "C"
Vec3 PhysExt_DebugGetPosition()
{
    return g_testPos;
}

#if 0
void Phys_CreateTestScene(ZBulletWorld* world)
{

    // hello bullet physics
    // Add static ground-plane
    g_physTest.groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), -1);
    g_physTest.groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, g_physTest.groundMotionState, g_physTest.groundShape, btVector3(0, 0, 0));
    groundRigidBodyCI.m_restitution = 1.0f;
    g_physTest.groundRigidBody = new btRigidBody(groundRigidBodyCI);
    world->dynamicsWorld->addRigidBody(g_physTest.groundRigidBody);


    // Add dynamic sphere, 50m above the ground
    g_physTest.sphereShape = new btSphereShape(1);
    g_physTest.sphereMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0.2f, 12, 0.5f)));
    
    btScalar mass = 1;
    btVector3 fallInertia(0, 0, 0);
    g_physTest.sphereShape->calculateLocalInertia(mass, fallInertia);

    btRigidBody::btRigidBodyConstructionInfo sphereBodyCI(mass, g_physTest.sphereMotionState, g_physTest.sphereShape, fallInertia);
    sphereBodyCI.m_restitution = 0.75f;
    sphereBodyCI.m_friction = 0.0f;
    
    g_physTest.sphereRigidBody = new btRigidBody(sphereBodyCI);
    //g_physTest.sphereRigidBody->setRestitution(1);
    world->dynamicsWorld->addRigidBody(g_physTest.sphereRigidBody);

}
#endif

/////////////////////////////////////////////////////////////
// Lifetime
/////////////////////////////////////////////////////////////

// If the provided command buffer is NULL or the size is zero, fail
// TODO: Could give the option to call function pointers for input/output
// instead of using buffers.
extern "C"
WorldHandle* PhysExt_Create(char* label, PhysErrorHandler errorHandler)
{
    printf("PHYS INIT\n");

    g_errorHandler = errorHandler;
    
    //g_physTest = {};
    ZBulletWorld* world = (ZBulletWorld*)malloc(sizeof(ZBulletWorld));
	ZE_ASSERT(world != NULL, "Failed to malloc physics world")
    *world = {};
    world->label = label;
    
    i32 bufferSize = MegaBytes(1);
	void* ptr = malloc(bufferSize);
	ZE_ASSERT(ptr != NULL, "Malloc failed")
    world->input = Buf_FromMalloc(ptr, bufferSize);
	ptr = malloc(bufferSize);
	ZE_ASSERT(ptr != NULL, "Malloc failed")
    world->output = Buf_FromMalloc(ptr, bufferSize);

	//world.verbose = 1;
    i32 bodiesMemorySize = sizeof(PhysBodyHandle) * PHYS_MAX_BODIES;
    u8* bodiesPtr = (u8*)malloc(bodiesMemorySize);
    ZE_SET_ZERO(bodiesPtr, bodiesMemorySize);
    world->bodies.items = (PhysBodyHandle*)bodiesPtr;
    world->bodies.capacity = PHYS_MAX_BODIES;

    world->overlapPairs = (PhysOverlapPair*)malloc(
        sizeof(PhysOverlapPair) * PHYS_MAX_OVERLAPS
    );
    world->numOverlaps = 0;
    world->maxOverlaps = PHYS_MAX_OVERLAPS;

    world->broadphase = new btDbvtBroadphase();

    world->collisionConfiguration = new btDefaultCollisionConfiguration();
    world->dispatcher = new btCollisionDispatcher(world->collisionConfiguration);

    world->solver = new btSequentialImpulseConstraintSolver();

    world->dynamicsWorld = new btDiscreteDynamicsWorld(
        world->dispatcher,
        world->broadphase,
        world->solver,
        world->collisionConfiguration);
    
    // User data is our new handle so that callbacks from bullet
    // can relocate it
    world->dynamicsWorld->setWorldUserInfo((void*)world);
	void* worldInfoPtr = world->dynamicsWorld->getWorldUserInfo();
	ZE_ASSERT(worldInfoPtr != NULL, "Got no world user info back!")

    world->dynamicsWorld->setGravity(btVector3(0, -20, 0));

	world->dynamicsWorld->setInternalTickCallback(
        Phys_PreSolveCallback,
        worldInfoPtr,
        true);
    world->dynamicsWorld->setInternalTickCallback(
        Phys_PostSolveCallback,
        worldInfoPtr,
        false);
    
    return &world->header;
    //Phys_CreateTestScene(&g_world);
}

extern "C"
void PhysExt_ClearWorld(WorldHandle* handle)
{
    HANDLE2WORLD
    if (world->verbose)
    {
        printf("PHYS Clear world\n");
    }
	Phys_ClearOverlapPairs(world);
    world->input.Clear(NO);
    world->output.Clear(NO);
    for (int i = 0; i < world->bodies.capacity; ++i)
    {
        Phys_FreeHandle(world, &world->bodies.items[i]);
    }
}

extern "C"
void PhysExt_Shutdown(WorldHandle* handle)
{
    HANDLE2WORLD
    PhysExt_ClearWorld(handle);
    // Get order right or it will cause an access violation
	delete world->dynamicsWorld;
	delete world->solver;
	delete world->dispatcher;
	delete world->collisionConfiguration;
    delete world->broadphase;
}

extern "C"
ZEByteBuffer PhysExt_Step(WorldHandle* handle, timeFloat deltaTime)
{
    HANDLE2WORLD
    
    Phys_LockCommandBuffer(&world->input);
    u32 reading = world->input.cursor - world->input.start;
    if (reading > world->debug.inputPeakBytes)
    {
        world->debug.inputPeakBytes = reading;
    }
	if (world->verbose)
	{
		printf("PHYS Reading %d bytes\n", reading);
	}

    // Reset output
    world->output.cursor = world->output.start;
    
    // So commands both read and write... Is this going to work?
    Phys_ReadCommands(world, &world->output);

    Phys_StepWorld(world, (f32)deltaTime);
    u32 written = world->output.cursor - world->output.start;
    if (written > world->debug.outputPeakBytes)
    {
        world->debug.outputPeakBytes = written;
    }
	if (world->verbose)
	{
		printf("PHYS Wrote %d bytes\n", written);
	}

    // TODO: Should only write the debug string if is asked for!
    Phys_WriteDebugOutput(world);
    ZEByteBuffer buf = world->output;
    return buf;
}


/*

Example of deleting full physics world:

https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=1660

"When a rigid body is added to the world, you should not delete it, until you after you removed it.
In general, delete objects in reverse order of creation, for example see BasicDemo: "

void	BasicDemo::exitPhysics()
{
	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	int i;
	for (i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}

	//delete collision shapes
	for (int j=0;j<m_collisionShapes.size();j++)
	{
		btCollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}

	//delete dynamics world
	delete m_dynamicsWorld;

	//delete solver
	delete m_solver;

	//delete broadphase
	delete m_overlappingPairCache;

	//delete dispatcher
	delete m_dispatcher;

	delete m_collisionConfiguration;

	
}
*/

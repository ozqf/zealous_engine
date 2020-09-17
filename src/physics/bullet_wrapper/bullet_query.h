#pragma once

#include "bullet_module.cpp"

#if 0
// Execute a raycast and write a results object + hits array to a byte buffer
// Return bytes written
i32 Phys_ExecRaycast_CmdBased(ZBulletWorld *world, PhysCmd_Raycast* cmd, u8* buf, u32 bufferCapacity)
{
    // prepare buffer header. cannot write it until the end when results are know.
    // record write pos and step, write later
    // record start position to calculate bytes written by the end
    u8* ptrStart = buf;
    u8* ptrHeaderWrite = buf;
    buf += sizeof(PhysDataItemHeader) + sizeof(PhysEv_RaycastResult);

    // Prepare results header
    PhysEv_RaycastResult result = {};
    result.queryId = cmd->id;
    result.start[0] = cmd->start[0];
    result.start[1] = cmd->start[1];
    result.start[2] = cmd->start[2];

    result.end[0] = cmd->end[0];
    result.end[1] = cmd->end[1];
    result.end[2] = cmd->end[2];

    result.numHits = 0;

    btVector3 start(result.start[0], result.start[1], result.start[2]);
    btVector3 end(result.end[0], result.end[1], result.end[2]);
    
    btCollisionWorld::AllHitsRayResultCallback rayCallback(start, end);
    world->dynamicsWorld->rayTest(start, end, rayCallback);
    i32 numHits = rayCallback.m_collisionObjects.size();
    
    for (i32 i = 0; i < numHits; ++i)
    {
        //const btCollisionObject* obj = rayCallback.m_collisionObjects[i];
        //btVector3 pos = rayCallback.m_hitPointWorld[i];
        //btVector3 normal = rayCallback.m_hitPointWorld[i];
        //printf("Hit at %.2f, %.2f, %.2f\n", pos.getX(), pos.getY(), pos.getZ());

        PhysRayHit hit = {};
        hit.worldPos[0] = rayCallback.m_hitPointWorld[i].getX();
        hit.worldPos[1] = rayCallback.m_hitPointWorld[i].getY();
        hit.worldPos[2] = rayCallback.m_hitPointWorld[i].getZ();

        hit.normal[0] = rayCallback.m_hitNormalWorld[i].getX();
        hit.normal[1] = rayCallback.m_hitNormalWorld[i].getY();
        hit.normal[2] = rayCallback.m_hitNormalWorld[i].getZ();
        buf += ZE_COPY_STRUCT(&hit, buf, PhysRayHit);
        result.numHits++;
    }
    
    PhysDataItemHeader h = {};
    h.type = RaycastResult;
    h.size = sizeof(PhysEv_RaycastResult) + (sizeof(PhysRayHit) * result.numHits);

    ptrHeaderWrite += ZE_COPY_STRUCT(&h, ptrHeaderWrite, PhysDataItemHeader);
    ptrHeaderWrite += ZE_COPY_STRUCT(&result, ptrHeaderWrite, PhysEv_RaycastResult);

    return (buf - ptrStart);
}
#endif

// Return number of hits
i32 Phys_QuickRaycast(ZBulletWorld *world, PhysCmd_Raycast* cmd, PhysRayHit* hits, i32 maxHits)
{
    btVector3 start(cmd->start[0], cmd->start[1], cmd->start[2]);
    btVector3 end(cmd->end[0], cmd->end[1], cmd->end[2]);

    btCollisionWorld::AllHitsRayResultCallback rayCallback(start, end);
    rayCallback.m_collisionFilterGroup = cmd->group;
    rayCallback.m_collisionFilterMask = cmd->mask;
    world->dynamicsWorld->rayTest(start, end, rayCallback);
    i32 numHits = rayCallback.m_collisionObjects.size();

    if (hits == NULL && maxHits <= 0) { return numHits; }
    i32 writtenHits = 0;
    for (i32 i = 0; i < numHits; ++i)
    {
        //btCollisionObject* obj = rayCallback.m_collisionObjects[i];
        hits->shapeId = rayCallback.m_collisionObjects[i]->getUserIndex();

        hits->worldPos[0] = rayCallback.m_hitPointWorld[i].getX();
        hits->worldPos[1] = rayCallback.m_hitPointWorld[i].getY();
        hits->worldPos[2] = rayCallback.m_hitPointWorld[i].getZ();

        hits->normal[0] = rayCallback.m_hitNormalWorld[i].getX();
        hits->normal[1] = rayCallback.m_hitNormalWorld[i].getY();
        hits->normal[2] = rayCallback.m_hitNormalWorld[i].getZ();
        ++hits;
        ++writtenHits;
        if (writtenHits == maxHits) { break; }
    }
    return numHits;
}

i32 PhysCmd_RayTest(
    ZBulletWorld *world,
    f32 x0, f32 y0, f32 z0,
    f32 x1, f32 y1, f32 z1)
{
    btVector3 start(x0, y0, z0);
    btVector3 end(x1, y1, z1);
    btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);

    world->dynamicsWorld->rayTest(start, end, rayCallback);

    if (rayCallback.hasHit())
    {
        btVector3 dir(x1 - x0, y1 - y0, z1 - z0);
        dir.normalize();
        const btCollisionObject* obj = rayCallback.m_collisionObject;
        #if 0
        if (!obj->isStaticOrKinematicObject())
        {
            //const btRigidBody *body = btRigidBody::upcast(obj);
            btRigidBody *foo = const_cast<btRigidBody *>(btRigidBody::upcast(obj));
            if (foo != 0)
            {
                Phys_SetBodyVelocity(foo, dir.getX() * 5, dir.getY() * 5, dir.getZ() * 5);
            }
        }
        #endif
    }

    return world->nextQueryId++;
}

#define GROUND_CHECK_EPSILON 0.2f

u8 PhysCmd_GroundTest(
    ZBulletWorld *world,
    f32 x0, f32 y0, f32 z0,
    f32 halfHeight,
    PhysEv_RaycastDebug* ev)
{
    //f32 halfHeight = (1.85f / 2.0f);

    f32 y1 = y0 -  halfHeight - GROUND_CHECK_EPSILON;
    btVector3 start(x0, y0, z0);
    btVector3 end(x0, y1, z0);
    if (ev != NULL)
    {
        ev->a[0] = x0;
        ev->a[1] = y0;
        ev->a[2] = z0;

        ev->b[0] = x0;
        ev->b[1] = y1;
        ev->b[2] = z0;
    }
    btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);

    world->dynamicsWorld->rayTest(start, end, rayCallback);

    if (rayCallback.hasHit())
    {
        if (ev != NULL)
        {
            ev->colour[0] = 1;
            ev->colour[1] = 0;
            ev->colour[2] = 0;
        }
        return 1; 
        #if 0
        btVector3 dir(x1 - x0, y1 - y0, z1 - z0);
        dir.normalize();
        const btCollisionObject* obj = rayCallback.m_collisionObject;
        #endif
    }
    if (ev != NULL)
        {
            ev->colour[0] = 0;
            ev->colour[1] = 1;
            ev->colour[2] = 0;
        }
    return 0;
}


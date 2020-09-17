
#pragma once

#include "bullet_module.cpp"

////////////////////////////////////////////////////////////////////////////////////////////
// CREATE SHAPES
////////////////////////////////////////////////////////////////////////////////////////////

PhysBodyHandle* Phys_CreateBulletBox(ZBulletWorld* world, ZShapeDef* def, ZCollisionBoxData* box)
{
    PhysBodyHandle* handle = Phys_GetHandleById(&world->bodies, def->handleId);
    ZE_ASSERT(handle != NULL, "No handle found");
    
    handle->shape = new btBoxShape(btVector3(box->halfSize[0], box->halfSize[1], box->halfSize[2]));
    handle->motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(def->pos[0], def->pos[1], def->pos[2])));
    //handle->mask = def->mask;
    //handle->group = def->group;
    if (world->verbose)
    {
        printf("PHYS New shape: shapeId %d, ownerId %d, group %d, mask %d, tag %d\n",
            handle->id, handle->externalId, def->mask, def->group, def->tag
        );
    }
    
    handle->def = *def;
    def = &handle->def;
    
    btScalar mass;
    if (def->flags & ZCOLLIDER_FLAG_STATIC)
    {
        mass = 0;
    }
    else
    {
        mass = 1;
    }

    btVector3 fallInertia(0, 0, 0);
    handle->shape->calculateLocalInertia(mass, fallInertia);

    btRigidBody::btRigidBodyConstructionInfo boxBodyCI(mass, handle->motionState, handle->shape, fallInertia);
    //boxBodyCI.m_restitution = PHYS_DEFAULT_RESTITUTION;
    //boxBodyCI.m_friction = PHYS_DEFAULT_FRICTION;
    
    handle->rigidBody = new btRigidBody(boxBodyCI);

    i32 btFlags = 0;
    if (def->flags & ZCOLLIDER_FLAG_KINEMATIC)
    {
        btFlags |= CF_KINEMATIC_OBJECT;
        // Restrict motion to specific axes (in this case, only move on X/Z)
        //handle->rigidBody->setLinearFactor(btVector3(1, 0, 1));
    }
    if (def->flags & ZCOLLIDER_FLAG_NO_COLLISION_RESPONSE)
    {
        // disable collision respones for ghost objects, triggers etc.
        btFlags |= CF_NO_CONTACT_RESPONSE;
    }
    if (def->flags & ZCOLLIDER_FLAG_NO_ROTATION)
    {
        handle->rigidBody->setAngularFactor(btVector3(0, 0, 0));
    }
    if (def->flags & ZCOLLIDER_FLAG_NO_ROTATION)
    {
        handle->rigidBody->setAngularFactor(btVector3(0, 0, 0));
    }
    handle->rigidBody->setRestitution(def->restitution);
    
    handle->rigidBody->setUserIndex(handle->id);
    handle->rigidBody->setCollisionFlags(btFlags);

    world->dynamicsWorld->addRigidBody(handle->rigidBody, def->group, def->mask);

    // Create Ghost Object
    // http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Collision_Callbacks_and_Triggers
    // https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=7468
    // http://bulletphysics.org/Bullet/BulletFull/classbtPairCachingGhostObject.html
    // https://www.gamedev.net/forums/topic/692573-bullet-btghostobject/
    //handle->ghost = new btGhostObject();
    //handle->ghost->setCollisionShape(new btBoxShape(btVector3(box->halfSize[0], box->halfSize[1], box->halfSize[2])));
    //world->dynamicsWorld->addCollisionObject(handle->ghost);

    return handle;
}
#if 0
PhysBodyHandle* Phys_CreateBulletSphere(ZBulletWorld* world, ZSphereDef def)
{
    PhysBodyHandle* handle = PHYS_GetFreeBodyHandle(&world->bodies);
    ZE_ASSERT(handle != NULL, "No handle found");
    /*
    u8 inUse;
    i32 id;
    btCollisionShape* shape;
    btDefaultMotionState* motionState;
    btRigidBody* rigidBody;
    */
    handle->shape = new btSphereShape(def.radius);
    handle->motionState = new btDefaultMotionState(
        btTransform(
            btQuaternion(0, 0, 0, 1),
            btVector3(def.base.pos[0], def.base.pos[1], def.base.pos[2])
            )
        );
    handle->mask = def.base.mask;
    
    
    btScalar mass;
    if (def.base.flags & ZCOLLIDER_FLAG_STATIC)
    {
        mass = 0;
    }
    else
    {
        mass = 1;
    }
    btVector3 fallInertia(0, 0, 0);
    handle->shape->calculateLocalInertia(mass, fallInertia);
    
    btRigidBody::btRigidBodyConstructionInfo sphereBodyCI(mass, handle->motionState, handle->shape, fallInertia);
    //sphereBodyCI.m_restitution = PHYS_DEFAULT_RESTITUTION;
    //sphereBodyCI.m_friction = PHYS_DEFAULT_FRICTION;

    i32 btFlags = 0;
    if (def.base.flags & ZCOLLIDER_FLAG_KINEMATIC)
    {
        btFlags |= CF_KINEMATIC_OBJECT;
    }
    if (def.base.flags & ZCOLLIDER_FLAG_NO_ROTATION)
    {
        handle->rigidBody->setAngularFactor(btVector3(0, 0, 0));
    }

    handle->rigidBody = new btRigidBody(sphereBodyCI);
    handle->rigidBody->setUserIndex(handle->id);
    handle->rigidBody->setCollisionFlags(btFlags);
    world->dynamicsWorld->addRigidBody(handle->rigidBody, def.base.mask, def.base.mask);

    // DONE
#if 0
    // TODO: Remove me: Set angular v test
    btVector3 v(0, 0, 0);
    handle->rigidBody->setLinearVelocity(v);
    // x = roll, y = pitch, z = yaw
    btVector3 angularV(0, 2, 0);
    handle->rigidBody->setAngularVelocity(angularV);
#endif
    // Create sensor
    // btGhostObject* ghost = new btGhostObject();
    // ghost->setCollisionShape(new btSphereShape(radius));
    // ghost->setWorldTransform(btTransform());

    // Init Ghost object for collision detection
    
    // handle->inUse;
    // handle->id;
    // handle->shape;
    // handle->motionState;
    // handle->rigidBody;

    return handle;
}
#endif
#if 0
PhysBodyHandle* Phys_CreateBulletInfinitePlane(ZBulletWorld* world, ZShapeDef def)
{
    PhysBodyHandle* handle = PHYS_GetFreeBodyHandle(&world->bodies);
    ZE_ASSERT(handle != NULL, "No free body handles");

    handle->shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    handle->motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, def.pos[1], 0)));
    handle->mask = def.mask;
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, handle->motionState, handle->shape, btVector3(0, 0, 0));
    //groundRigidBodyCI.m_restitution = PHYS_DEFAULT_RESTITUTION;
    //groundRigidBodyCI.m_friction = PHYS_DEFAULT_FRICTION;
    handle->rigidBody = new btRigidBody(groundRigidBodyCI);
    world->dynamicsWorld->addRigidBody(handle->rigidBody, def.mask, def.mask);
    handle->rigidBody->setUserIndex(handle->id);
    return handle;
}
#endif
internal void PhysExec_CreateShape(ZBulletWorld* world, ZShapeDef* def)
{
     switch (def->shapeType)
    {
        case box:
        {
            Phys_CreateBulletBox(world, def, &def->data.box);
        } break;

        default:
        {
            ILLEGAL_CODE_PATH
        } break;
    }
}


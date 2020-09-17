#pragma once

#include "bullet_module.cpp"

////////////////////////////////////////////////////////////////////////////////////////////
// MANAGE HANDLES
////////////////////////////////////////////////////////////////////////////////////////////
internal PhysBodyHandle *Phys_GetFreeBodyHandle(PhysBodyList *list)
{

    for (i32 i = 0; i < list->capacity; ++i)
    {
        PhysBodyHandle *handle = &list->items[i];
        if (handle->inUse == FALSE)
        {
            handle->id = i;
            handle->inUse = TRUE;
            return handle;
        }
    }
    return NULL;
}

internal PhysBodyHandle *Phys_GetHandleById(PhysBodyList *list, i32 queryId)
{
    ZE_ASSERT(queryId >= 0, "Query Id < 0");
    ZE_ASSERT(queryId < list->capacity, "Query Id out of bounds");
    // Handles that have not been 'issued' have no business being found!
	if (list->items[queryId].inUse == FALSE) { return NULL; }
    //Assert(list->items[queryId].inUse);
    return &list->items[queryId];
}

// Delete all objects on this handle
internal void Phys_FreeHandle(ZBulletWorld *world, PhysBodyHandle *handle)
{
    // TODO: This is almost certainly NOT how to free stuff in bullet
    // !find proper example!
    // Assuming that on shutdown we will be removing freeing
    // shape handles that were never initialised to begin with
    if (handle->rigidBody != NULL)
    {
        world->dynamicsWorld->removeRigidBody(handle->rigidBody);
        delete handle->rigidBody;
    }
    if (handle->motionState != NULL)
    {
        delete handle->motionState;
    }
    if (handle->shape != NULL)
    {
        delete handle->shape;
    }

    handle->inUse = FALSE;
    *handle = {};
}

/**
 * deactivate all objects on this handle and mark it for reuse
 */
/*
internal void Phys_RecycleHandle(ZBulletWorld *world, PhysBodyHandle *handle)
{
    handle->inUse = FALSE;
}
*/
// TODO: Remove me I live in constant agony
// Hack because I want these functions to be internal
// but I also want to avoid the compile time warning in this specific case
void Phys_NeverCall()
{
    ILLEGAL_CODE_PATH
    //Phys_CreateBulletBox(NULL, NULL, NULL);
    //PhysExec_CreateShape(NULL, NULL);
    //Phys_RecycleHandle(NULL, NULL);
    //Phys_ReadCommands(NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////
// COMMANDS
////////////////////////////////////////////////////////////////////////////////////////////

// Add/remove from the simulation without deleting the shape
void PhysExec_SetBodyActiveState(btRigidBody *body, i32 bIsActive)
{
    body->setActivationState(bIsActive);
}

void PhysExec_SetBodyPosition(btRigidBody *body, f32 x, f32 y, f32 z)
{
    btTransform t;
    body->getMotionState()->getWorldTransform(t);
    btVector3 newPosition = t.getOrigin();
    newPosition.setX(-newPosition.getX() + x);
    newPosition.setY(-newPosition.getY() + y);
    newPosition.setZ(-newPosition.getZ() + z);
    body->translate(newPosition);
}

void Phys_SetBodyVelocity(btRigidBody *body, f32 vx, f32 vy, f32 vz)
{
    body->activate(true);
    btVector3 newVelocity;
    newVelocity.setX(vx);
    newVelocity.setY(vy);
    newVelocity.setZ(vz);
    body->setLinearVelocity(newVelocity);
}

internal void PhysExec_TeleportShape(ZBulletWorld *world, PhysCmd_Teleport *cmd)
{
    PhysBodyHandle *handle = Phys_GetHandleById(&world->bodies, cmd->shapeId);
    if (handle == NULL)
    {
        return;
    }
    handle->rigidBody->activate(true);
    PhysExec_SetBodyPosition(
        handle->rigidBody,
        cmd->pos[0],
        cmd->pos[1],
        cmd->pos[2]
    );
#if 0
    btTansform t;
    handle->rigidBody->getMotionState()->getWorldTransform(t);
    btVector3 newPosition = t.getOrigin();
    newPosition.setX(-newPosition.getX() + cmd->pos[0]);
    newPosition.setY(-newPosition.getY() + cmd->pos[1]);
    newPosition.setZ(-newPosition.getZ() + cmd->pos[2]);
    /*handle->rigidBody->translate(btVector3(cmd->pos[0], cmd->pos[1], cmd->pos[2]));*/
    handle->rigidBody->translate(newPosition);
    //btMotionState state = handle->rigidBody->getMotionState();
#endif
}

internal void PhysExec_ChangeVelocity(ZBulletWorld *world, PhysCmd_VelocityChange *cmd)
{
    PhysBodyHandle *handle = Phys_GetHandleById(&world->bodies, cmd->shapeId);
    if (handle == NULL)
    {
        return;
    }
    if (handle->def.flags & ZCOLLIDER_FLAG_STATIC)
    {
        printf("PHYS Attempting to Change velocity of static body %d\n", cmd->shapeId);
        return;
    }
    Phys_SetBodyVelocity(handle->rigidBody, cmd->vel[0], cmd->vel[1], cmd->vel[2]);
    /*handle->rigidBody->activate(true);
	btVector3 newVelocity;
	newVelocity.setX(cmd->vel[0]);
	newVelocity.setY(cmd->vel[1]);
	newVelocity.setZ(cmd->vel[2]);
	handle->rigidBody->setLinearVelocity(newVelocity);*/
}

internal void PhysExec_SetState(ZBulletWorld* world, PhysCmd_State *cmd)
{
    PhysBodyHandle *handle = Phys_GetHandleById(&world->bodies, cmd->shapeId);
    if (handle == NULL)
    {
        return;
    }
    handle->rigidBody->activate(true);
    btTransform t;
}

////////////////////////////////////////////////////////////////////////////////////////////
// LOOP
////////////////////////////////////////////////////////////////////////////////////////////
internal void Phys_LockCommandBuffer(ZEByteBuffer *buffer)
{
    *buffer->cursor = 0;
}

internal void Phys_ReadCommands(ZBulletWorld *world, ZEByteBuffer* output)
{
    //Phys_TickCallback(g_world.dynamicsWorld, 0.016f);
    ZEByteBuffer *buffer = &world->input;
    u8 *ptrRead = buffer->start;
    u8* end = buffer->cursor;
	i32 executed = 0;
    while (*ptrRead != NULL && ptrRead < end)
    {
        u8 cmdType = COM_ReadByte(&ptrRead);
        switch (cmdType)
        {
            case Teleport:
            {
				++executed;
                PhysCmd_Teleport cmd = {};
                ptrRead += ZE_COPY_STRUCT(ptrRead, &cmd, PhysCmd_Teleport);
                PhysExec_TeleportShape(world, &cmd);
            } break;

            case SetVelocity:
            {
				++executed;
                PhysCmd_VelocityChange cmd = {};
                ptrRead += ZE_COPY_STRUCT(ptrRead, &cmd, PhysCmd_VelocityChange);
                PhysExec_ChangeVelocity(world, &cmd);
            } break;

            case SetState:
            {
				++executed;
                PhysCmd_State cmd = {};
                ptrRead += ZE_COPY_STRUCT(ptrRead, &cmd, PhysCmd_State);
                PhysExec_SetState(world, &cmd);
            } break;

            case Create:
            {
				++executed;
                ZShapeDef def = {};
                ptrRead += ZE_COPY_STRUCT(ptrRead, &def, ZShapeDef);
                PhysExec_CreateShape(world, &def);
            } break;

            case Remove:
            {
				++executed;
                i32 shapeId = COM_ReadI32(&ptrRead);
                PhysBodyHandle* h = Phys_GetHandleById(&world->bodies, shapeId);
                if (h == NULL)
                {
                    printf("PHYS: Not shape %d to remove!\n", shapeId);
                    break;
                }
                else
                {
                    //printf("PHYS Removed shape %d\n", shapeId);
                    Phys_FreeHandle(world, h);
                }
            } break;

            // TODO:
            // So how is this going to work exactly...?
            #if 0
            case Raycast:
            {
                PhysCmd_Raycast cmd = {};
                ptrRead += ZE_COPY_STRUCT(ptrRead, &cmd, PhysCmd_Raycast);
                // needs output buffer for results...
                Phys_ExecRaycast(world, &cmd, output);
            }
            break
            #endif;

            default:
            {
                ILLEGAL_CODE_PATH
            } break;
        }
    }
	if (world->verbose)
	{
		printf("  Executed %d cmds\n", executed);
	}
    // Clear
    buffer->Clear(NO);
    COM_WriteByte(0, buffer->start);
    // reset buffer write position
    buffer->cursor = buffer->start;
}

internal void Phys_StepWorld(ZBulletWorld *world, f32 deltaTime)
{
    ++world->debug.stepCount;
	ZE_ASSERT(world->dynamicsWorld->getWorldUserInfo() != NULL, "Null user info!")
    world->dynamicsWorld->stepSimulation(deltaTime, 10, deltaTime);

    u8 *writePosition = world->output.cursor;
	u8 *endPosition = world->output.start + world->output.capacity;
    i32 len = world->bodies.capacity;
	i32 updatesWritten = 0;
	i32 unusedSkipped = 0;
	i32 inactiveSkipped = 0;
    for (int i = 0; i < len; ++i)
    {
        PhysBodyHandle *handle = &world->bodies.items[i];
        // Check that a rigidbody is present and that the handle is in use.
        // Otherwise it might be a rigidbody await recylcing
        if (handle->inUse == FALSE || handle->rigidBody == NULL)
        {
			unusedSkipped++;
            continue;
        }
        if (!handle->rigidBody->getActivationState())
        {
			inactiveSkipped++;
            continue;
        }
        ZE_ASSERT(handle->motionState != NULL, "Motion state is null");
        i32 requiredSize = sizeof(PhysEV_TransformUpdate);
        if (writePosition +  requiredSize > endPosition)
        {
            Phys_Error("Output Buffer overrun\n");
        }
		//Assert((writePosition +  requiredSize < endPosition));

		updatesWritten++;

        
        PhysEV_TransformUpdate updateEv = {};
        // TODO: If updates vary in size in the future (and they are big,
        // what with containing an entire matrix and all) this will have to be set
        // AFTER the update is written to the buffer
        PhysEv_SetHeader(&updateEv.header, TransformUpdate, sizeof(PhysEV_TransformUpdate));
        
        
        updateEv.ownerId = handle->externalId;
        updateEv.tag = handle->def.tag;
        
        btTransform t;
        handle->rigidBody->getMotionState()->getWorldTransform(t);

        btScalar openglM[16];
        t.getOpenGLMatrix(openglM);
        for (i32 j = 0; j < 16; ++j)
        {
            updateEv.matrix[j] = openglM[j];
        }

        btVector3 vel = handle->rigidBody->getLinearVelocity();
        updateEv.vel[0] = vel.getX();
        updateEv.vel[1] = vel.getY();
        updateEv.vel[2] = vel.getZ();

        if ((
            handle->def.flags & ZCOLLIDER_FLAG_NO_ROTATION) ||
            handle->def.flags & ZCOLLIDER_FLAG_GROUNDCHECK)
        {
            PhysEv_RaycastDebug rayEv = {};
            // TODO: Assuming handle is for a box and halfsize is valid!
            u8 val = PhysCmd_GroundTest(
                world, openglM[M4x4_W0], openglM[M4x4_W1], openglM[M4x4_W2],
                handle->def.data.box.halfSize[1],
                &rayEv);
            if (val)
            {
                updateEv.flags |= PHYS_EV_FLAG_GROUNDED;
            }
            
            // Write ground check debug
            //dataHeader.type = RaycastDebug;
            //dataHeader.size = sizeof(PhysEv_RaycastDebug);
            //writePosition += ZE_COPY_STRUCT(&dataHeader, writePosition, PhysDataItemHeader);
            //writePosition += ZE_COPY_STRUCT(&rayEv, writePosition, PhysEv_RaycastDebug);
        }
        // Write update to buffer
        writePosition += ZE_COPY(&updateEv, writePosition, updateEv.header.size);
    }

    // Mark end of buffer
    *writePosition = 0;
    world->output.cursor = writePosition;

    /*char buf[128];
        sprintf_s(buf, 128, "Sphere pos: %.2f, %.2f, %.2f\n", pos.getX(), pos.getY(), pos.getZ());
        OutputDebugString(buf);*/
}

void OldDefunctWriteUpdateCode()
{

        /*btVector3 pos = t.getOrigin();
		tUpdate.matrix[12] = pos.x();
		tUpdate.matrix[13] = pos.y();
		tUpdate.matrix[14] = pos.z();*/

#if 0 // Copy rotation via calculating euler angles in opengl matrix
		t.getOpenGLMatrix(openglM);
		
		f32 fAngZ = atan2f(openglM[1], openglM[5]);
		f32 fAngY = atan2f(openglM[8], openglM[10]);
		f32 fAngX = -asinf(openglM[9]);
		tUpdate.rot[0] = fAngX;// *RAD2DEG;
		tUpdate.rot[1] = fAngY;// * RAD2DEG;
		tUpdate.rot[2] = fAngZ;// * RAD2DEG;
#endif

#if 0 // Rotation copy to matrix via euler angles into bottom row
		btMatrix3x3& rot = t.getBasis();
		btScalar yaw, pitch, roll;
		rot.getEulerYPR(yaw, pitch, roll);
		tUpdate.matrix[3] = roll;
		tUpdate.matrix[7] = pitch;
		tUpdate.matrix[11] = yaw;
#endif

#if 0 // Rotation copy to matrix directly
		btMatrix3x3& rot = t.getBasis();
		btVector3 xAxis = rot.getColumn(0);
		btVector3 yAxis = rot.getColumn(1);
		btVector3 zAxis = rot.getColumn(2);

		tUpdate.matrix[0] = xAxis.x();
		tUpdate.matrix[1] = xAxis.y();
		tUpdate.matrix[2] = xAxis.z();

		tUpdate.matrix[4] = yAxis.x();
		tUpdate.matrix[5] = yAxis.y();
		tUpdate.matrix[6] = yAxis.z();

		tUpdate.matrix[8] = zAxis.x();
		tUpdate.matrix[9] = zAxis.y();
		tUpdate.matrix[10] = zAxis.z();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG
////////////////////////////////////////////////////////////////////////////////////////////
internal void Phys_WriteDebugOutput(ZBulletWorld *world)
{
    char *ptr = g_debugString;
    i32 remaining = g_debugStringSize;
    i32 written = 0;

    written = sprintf_s(ptr, remaining,
                        "Physics debug\n\
Steps: %d\n\
PreSolves: %d\n\
PostSolves: %d\n\
Num Manifolds: %d\n\
Num Overlaps: %d\n\
Peak input bytes: %d\n\
Peak output bytes: %d\n\
--Collision Pairs--\n\
",
                        world->debug.stepCount,
                        world->debug.preSolves,
                        world->debug.postSolves,
                        world->debug.numManifolds,
                        world->numOverlaps,
                        world->debug.inputPeakBytes,
                        world->debug.outputPeakBytes
                        );
      ptr += written;
      remaining -= written;

	  char* format = "(%d vs %d)\n";
	  // length of format + 4 digits per shape id
	  i32 lineLengthEstimate = 9 + 4 + 4;

      for (int i = 0; i < PHYS_MAX_OVERLAPS; ++i)
      {
		  if (remaining < lineLengthEstimate)
		  {
              // TODO: Tidy this function up
			  //printf("ZPHYS Debug line overflow, %d remaining\n", remaining);
			  break;
		  }
          PhysOverlapPair* pair = &world->overlapPairs[i];
          if (!pair->isActive) { continue; }
          written = sprintf_s(ptr, remaining, format, pair->a.internalId, pair->b.internalId);
          //written = sprintf_s(ptr, remaining, "(%d vs %d)\n", 1, 2);
          //written = sprintf_s(ptr, remaining, "12345");
          ptr += written;
          remaining -= written;
      }
    ptr = NULL;
}

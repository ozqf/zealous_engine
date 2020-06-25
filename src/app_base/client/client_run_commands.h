
#include "client_internal.h"

#include "../../sys_events.h"
#include "../../sim/sim.h"

#if 0
internal void CL_ReadReliableCommands(NetStream* stream)
{
    ZEByteBuffer* b = &stream->inputBuffer;
    u8* read = b->start;
    u8* end = b->ptrEnd;
    while (read < end)
    {
        Command* header = (Command*)read;
        ZE_ASSERT(header->sentinel == CMD_SENTINEL)
        ZE_ASSERT(header->size > 0)
        read += header->size;
        switch (header->type)
        {
            case CMD_TYPE_IMPULSE:
            {
                CmdImpulse* cmd = (CmdImpulse*)header;
            } break;
            default:
            {
                printf("CL Unknown command type %d\n", header->type);
                ILLEGAL_CODE_PATH
            } break;
        }
    }
}
#endif
internal void CL_ReadSystemEvents(
	ZEByteBuffer* sysEvents, timeFloat deltaTime, frameInt platformFrame)
{
    AppTimer timer(APP_STAT_CL_INPUT, g_sim.tick);
    //printf("CL Reading platform events (%d bytes)\n", sysEvents->Written());
    u8* read = sysEvents->start;
    u8* end = sysEvents->cursor;
    while (read < end)
    {
        SysEvent* ev = (SysEvent*)read;
        i32 err = Sys_ValidateEvent(ev);
        if (err != ZE_ERROR_NONE)
        {
            printf("CL Error %d reading system event header\n", err);
            return;
        }
        read += ev->size;
        switch(ev->type)
        {
            case SYS_EVENT_PACKET:
            {
				SysPacketEvent* packet = (SysPacketEvent*)ev;
                CL_ReadPacket(
                    packet,
                    &g_reliableStream,
                    &g_unreliableStream,
                    &g_sim.quantise,
                    g_elapsed);
            } break;

            case SYS_EVENT_INPUT:
            {
                SysInputEvent* inputEv = (SysInputEvent*)ev;
                Input_TestForAction(
					&g_inputActions,
					inputEv->value,
					inputEv->inputID,
					platformFrame);
				
				if (Input_CheckActionToggledOn(
					&g_inputActions,
					"Menu",
					platformFrame))
				{
					g_mainMenuOn = !g_mainMenuOn;
				}

                // Check for debug
                // g_bVerboseFrame
                if (inputEv->inputID == Z_INPUT_CODE_O)
                {
                    printf("CL - post verbose frame\n");
                    g_bVerboseFrame = YES;
                }

            } break;
            case SYS_EVENT_SKIP: break;
        }
    }
}

internal i32 CL_IsCommandTickSensitive(i32 cmdType)
{
	switch (cmdType)
	{
		case CMD_TYPE_S2C_SESSION_SYNC: { return NO; }
	}
	return YES;
}

/**
 * Point of execution for the most important server messages.
 */
internal i32 CL_ExecReliableCommand(
    SimScene* sim, Command* h, timeFloat deltaTime, i32 tickDiff)
{
    //APP_LOG(64, "CL exec input seq %d\n", h->sequence);

	switch (h->type)
	{
        case CMD_TYPE_S2C_BULK_SPAWN:
        {
            S2C_BulkSpawn* prj = (S2C_BulkSpawn*)h;
            APP_LOG(256, "CL Spawn Prj %d on SV tick %d (local sv tick diff %d. Cmd tick %d)\n",
                prj->def.factoryType,
				prj->def.base.tick,
				prj->def.base.tick - CL_GetServerTick(),
				prj->header.tick
            );
            // flip diff to specify fast forwarding
            i32 flags;
            f32 priority;
            Sim_ExecuteBulkSpawn(sim, &prj->def, -tickDiff, &flags, &priority);
        } break;
		case CMD_TYPE_S2C_RESTORE_ENTITY:
		{
			S2C_RestoreEntity* spawn = (S2C_RestoreEntity*)h;
			APP_LOG(64, "CL Spawn %d at %.3f, %.3f, %.3f\n",
				spawn->networkId, spawn->pos.x, spawn->pos.y, spawn->pos.z
			);
			
			SimEntSpawnData def = {};
			def.serial = spawn->networkId;
            def.birthTick = h->tick;
            def.factoryType = spawn->factoryType;
			def.pos = spawn->pos;
			def.velocity = spawn->vel;
            def.fastForwardTicks = -tickDiff;
			Sim_RestoreEntity(sim, &def);
		} break;
        case CMD_TYPE_S2C_REMOVE_ENTITY:
        {
            S2C_RemoveEntity* cmd = (S2C_RemoveEntity*)h;
            CLG_HandleEntityDeath(&g_sim, cmd->entityId);
            Sim_RemoveEntity(sim, cmd->entityId);
            //APP_PRINT(64, "CL Remove Ent %d\n", cmd->entityId);
        } break;
        case CMD_TYPE_S2C_REMOVE_ENTITY_GROUP:
        {
            S2C_RemoveEntityGroup* cmd = (S2C_RemoveEntityGroup*)h;
            for (i32 i = 0; i < cmd->numIds; ++i)
            {
                i32 serial = cmd->firstId + i;
                CLG_HandleEntityDeath(sim, serial);
                Sim_RemoveEntity(sim, serial);
            }
        } break;
        case CMD_TYPE_S2C_SESSION_SYNC:
        {
            S2C_Sync* sync = (S2C_Sync*)h;
			CL_SetServerTick(sync->header.tick);
			g_avatarSerial = sync->avatarEntityId;
            g_clientState = CLIENT_STATE_PLAY;
            //i32 sceneId;
            APP_PRINT(64, "CL Sync - Set avatar %d\n", g_avatarSerial);
            // Lets not do what the server tells us!
			//g_serverTick = sync->simTick - sync->jitterTickCount;
			APP_LOG(64, "/////////////////////////////////////////\n");
            APP_LOG(64, "CL Sync server sim tick %d\n", CL_GetServerTick());
			APP_LOG(64, "/////////////////////////////////////////\n");
        } break;
		
		
		default:
		{
			APP_PRINT(64, "CL Unknown command type %d\n", h->type);
		} break;
	}
    return 1;
}

internal void CL_RunReliableCommands(
    SimScene* sim, NetStream* stream, timeFloat deltaTime)
{
	ZEByteBuffer* b = &stream->inputBuffer;
	
	for (;;)
	{
		i32 sequence = stream->inputSequence;
		Command* h = Stream_FindMessageBySequence(
			b->start, b->Written(), sequence);
        // No commands to run
		if (!h) { break; }

		i32 diff = h->tick - CL_GetServerTick();
		//APP_LOG(128, "CL Exec Cmd %d: Cmd Tick %d, Sync Tick %d (diff %d), CL Tick %d\n",
		//	h->sequence, h->tick, CL_GetServerTick(), diff, g_ticks
		//);
		
		if (CL_IsCommandTickSensitive(h->type))
		{
			if (diff > 0)
			{
				APP_LOG(128, "\tCL Delaying execution of cmd %d until tick %d (diff %d)\n",
					h->sequence, h->tick, diff);
				// Drop out - next reliable command cannot be executed until we reach this frame
				break;
			}
			
			if (diff < 0)
			{
				APP_LOG(128,  "\tCL Fast forward cmd %d by %d frames\n",
					h->sequence, -(diff)
				);
			}
		}
		
		// Step queue counter forward as we are now executing
		stream->inputSequence++;
		
		i32 err = Cmd_Validate(h);
		ZE_ASSERT(err == ZE_ERROR_NONE, "Invalid command")
		if (CL_ExecReliableCommand(sim, h, deltaTime, diff))
        {
            Stream_DeleteCommand(b, h, 0);
        }
	}
}

internal void CL_RunUnreliableCommands(
    SimScene* sim, NetStream* stream, timeFloat deltaTime)
{
	ZEByteBuffer* b = &stream->inputBuffer;
    u8* read = b->start;
	//APP_LOG(128, "CL Run %d bytes of unreliable msgs\n",
    //    b->Written());
    //CL_LogCommandBuffer(b, "unreliable");

	while (read < b->cursor)
	{
		i32 sequence = stream->inputSequence;
		Command* h = (Command*)read;
        i32 err = Cmd_Validate(h);
		if (err != ZE_ERROR_NONE)
		{
			APP_PRINT(128, "CL Run unreliable - invalid cmd code %d\n", err);
            b->Clear(NO);
			return;
		}
        
		i32 diff = h->tick - CL_GetServerTick();
		//APP_LOG(128, "CL Exec Cmd %d: Cmd Tick %d, Sync Tick %d (diff %d), CL Tick %d\n",
		//	h->sequence, h->tick, CL_GetServerTick(), diff, g_ticks
		//)
		
		// If executed,   delete command from buffer
        i32 executed = 0;

        // some sync events are related to server tick...
        if (h->type == CMD_TYPE_S2C_SYNC_ENTITY
            && diff <= 0)
        {
            S2C_EntitySync* cmd = (S2C_EntitySync*)h;
                executed = CLG_SyncEntity(sim, cmd);
        }
        // ...some should be executed immediately
        else if (h->type == CMD_TYPE_S2C_INPUT_RESPONSE)
        {
            S2C_InputResponse* cmd = (S2C_InputResponse*)h;

            #if 1
            CLI_RecordServerResponse(g_serverResponses, cmd);
            if (g_lastInputResponse.lastUserInputSequence < cmd->lastUserInputSequence)
            {
                // New input record
                g_lastInputResponse = *cmd;
                g_bHasNewResponse = YES;
            }

            #endif
            
            #if 0 // Old - execute immediately
            CLG_SyncAvatar(sim, cmd);
            CLI_RecordServerResponse(g_serverResponses, cmd);
            #endif
            executed = 1;
        }
        else if (h->type == CMD_TYPE_PING)
        {
            CmdPing* cmd = (CmdPing*)h;
            executed = 1;
        }

        if (executed)
        {
            Stream_DeleteCommand(b, h, 0);
        }
        else
        {
            read += h->size;
        }
    }
}

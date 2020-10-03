#include "sim_internal.h"

// TODO: Move sim related commands into sim module!
// #include "../app_base/shared/commands.h"

internal void Sim_ExecuteCommands(
	SimScene* sim,
    ZEBuffer* input,
	timeFloat delta)
{
	u8* read = input->start;
	u8* end = input->cursor;
	i32 tickDiff = 0;
	while (read < end)
	{
		ZECommand* h = (ZECommand*)read;
		i32 err = ZCmd_Validate(h);
		if (err != ZE_ERROR_NONE)
		{
			printf("Sim command validation error %d\n", err);
			return;
		}
		read += h->size;

		switch (h->type)
		{
			case SIM_CMD_TYPE_BULK_SPAWN:
        	{
        	    SimEvent_BulkSpawn* cmd = (SimEvent_BulkSpawn*)h;
        	    APP_LOG(256, "CL Spawn cmd %d on SV tick %d (local sv tick diff %d)\n",
        	        cmd->factoryType,
					cmd->base.tick,
					cmd->base.tick - sim->info.tick
        	    );
        	    // flip diff to specify fast forwarding
        	    i32 flags;
        	    f32 priority;
        	    Sim_ExecuteBulkSpawn(sim, cmd, -tickDiff, &flags, &priority);
        	} break;
			case SIM_CMD_TYPE_REMOVE_ENTITY:
        	{
        	    SimEvent_RemoveEnt* cmd = (SimEvent_RemoveEnt*)h;
				if (cmd->style != 0 && (sim->info.flags & SIM_SCENE_BIT_IS_CLIENT) != 0)
				{
					// spawn fx
					SimFx_EntityDeath(sim, cmd);
				}
        	    //CLG_HandleEntityDeath(&g_sim, cmd->entityId);
				Sim_RecycleEntity(sim, cmd->entityId);
        	    //Sim_RemoveEntity(sim, cmd->entityId);
        	    //APP_PRINT(64, "CL Remove Ent %d\n", cmd->entityId);
        	} break;
			case SIM_CMD_TYPE_RESTORE_ENTITY:
			{
				SimEvent_Spawn* cmd = (SimEvent_Spawn*)h;
				Sim_RestoreEntity(sim, cmd);
			} break;
			
			case SIM_CMD_TYPE_SYNC_ENTITY:
			{
				// TODO: Sim internal entity sync.
				// S2C_EntitySync* cmd = (S2C_EntitySync*)h;
                // CLG_SyncEntity(sim, cmd);
			} break;

			case SIM_CMD_TYPE_PLAYER_INPUT:
			{
				SimEvent_PlayerInput* cmd = (SimEvent_PlayerInput*)h;
				SimPlayer* plyr = SimPlyr_Get(sim, cmd->playerId);
				if (plyr == NULL)
				{
					printf("GAME - no player %d to set input\n", cmd->playerId);
					break;
				}
				plyr->input = cmd->input;
				// SimEntity* ent = Sim_GetEntityBySerial(sim, cmd->entityId);
				// if (ent == NULL)
				// {
				// 	printf("SIM - no entity %d to set actor input\n", cmd->entityId);
				// 	break;
				// }
				// ent->input = cmd->input;
			} break;
			
			case SIM_CMD_TYPE_PLAYER_STATE:
			{
				SimEvent_PlayerState* state = (SimEvent_PlayerState*)h;
				SimPlyr_UpdateState(sim, state);
			} break;
			#if 0
			case CMD_TYPE_S2C_RESTORE_ENTITY:
			{
				S2C_RestoreEntity* spawn = (S2C_RestoreEntity*)h;

				SimEvent_Spawn def = {};
				def.serial = spawn->networkId;
        	    def.birthTick = h->tick;
        	    def.factoryType = spawn->factoryType;
				def.pos = spawn->pos;
				def.velocity = spawn->vel;
        	    def.fastForwardTicks = -tickDiff;
				Sim_RestoreEntity(sim, &def);
			} break;
			case CMD_TYPE_C2S_INPUT:
			{
				// TODO: Set actor input requires entity id which server gets
				// from user, not from the command!
				// C2S_Input* cmd = (C2S_Input*)h;
				// Sim_SetActorInput(sim, &cmd->input, cmd->)
				// cmd->input
			} break;
			
			case CMD_TYPE_S2C_REMOVE_ENTITY_GROUP:
        	{
        	    S2C_RemoveEntityGroup* cmd = (S2C_RemoveEntityGroup*)h;
        	    for (i32 i = 0; i < cmd->numIds; ++i)
        	    {
        	        i32 serial = cmd->firstId + i;
        	        //CLG_HandleEntityDeath(sim, serial);
        	        Sim_RemoveEntity(sim, serial);
        	    }
        	} break;
			#endif
		}
	}
}
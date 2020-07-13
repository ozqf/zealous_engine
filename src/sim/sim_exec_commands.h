#include "sim_internal.h"

// TODO: Move sim related commands into sim module!
// #include "../app_base/shared/commands.h"

internal void Sim_ExecuteCommands(
	SimScene* sim,
    ZEByteBuffer* input,
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
        	    SimBulkSpawnEvent* cmd = (SimBulkSpawnEvent*)h;
        	    APP_LOG(256, "CL Spawn cmd %d on SV tick %d (local sv tick diff %d. Cmd tick %d)\n",
        	        cmd->def.factoryType,
					cmd->def.base.tick,
					cmd->def.base.tick - CL_GetServerTick(),
					cmd->header.tick
        	    );
        	    // flip diff to specify fast forwarding
        	    i32 flags;
        	    f32 priority;
        	    Sim_ExecuteBulkSpawn(sim, cmd, -tickDiff, &flags, &priority);
        	} break;
			#if 0
			case CMD_TYPE_S2C_SYNC_ENTITY:
			{
				// TODO: Sim internal entity sync.
				// S2C_EntitySync* cmd = (S2C_EntitySync*)h;
                // CLG_SyncEntity(sim, cmd);
			} break;
			case CMD_TYPE_S2C_RESTORE_ENTITY:
			{
				S2C_RestoreEntity* spawn = (S2C_RestoreEntity*)h;

				SimEntSpawnData def = {};
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
			case CMD_TYPE_S2C_REMOVE_ENTITY:
        	{
        	    S2C_RemoveEntity* cmd = (S2C_RemoveEntity*)h;
        	    //CLG_HandleEntityDeath(&g_sim, cmd->entityId);
        	    Sim_RemoveEntity(sim, cmd->entityId);
        	    //APP_PRINT(64, "CL Remove Ent %d\n", cmd->entityId);
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
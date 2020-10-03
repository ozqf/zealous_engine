#include "sim.h"

internal void Sim_RotateTitleCam(SimScene* sim, timeFloat delta)
{
    Transform* camera = &sim->info.observePos;
    f32 x = cosf((f32)sim->info.time / 2.f) * 30;
    f32 z = sinf((f32)sim->info.time / 2.f) * 30;
    camera->pos.x = x;
    camera->pos.y = 30;
    camera->pos.z = z;
    Vec3 target = sim->info.observeTarget;
    Vec3 toTarget =
    {
        target.x + camera->pos.x,
        target.y + camera->pos.y,
        target.z + camera->pos.z
    };
    Vec3_Normalise(&toTarget);
    Vec3 rot = Vec3_EulerAngles(toTarget);
    Transform_SetRotation(camera, rot.x, rot.y, rot.z);
}

internal void SimRules_PostUpdate(SimScene* sim, timeFloat delta)
{
    if ((sim->info.flags & SIM_SCENE_BIT_IS_SERVER) == 0) { return; }
    switch (sim->info.gameRules)
    {
        case SIM_GAME_RULES_SURVIVAL:
        if (sim->info.gameState == SIM_GAME_STATE_WARMUP)
        {
            Sim_RotateTitleCam(sim, delta);    
        }
        break;
        case SIM_GAME_RULES_NONE:
        Sim_RotateTitleCam(sim, delta);
        break;
    }
}

internal i32 SimRules_SpawnPlayer(SimScene* sim, SimPlayer* plyr)
{
    if (sim->info.gameState == SIM_GAME_STATE_GAME_OVER)
	{
		printf("SIM cannot spawn player - game over\n");
		return NO;
	}
    if (sim->info.gameRules != SIM_GAME_RULES_SURVIVAL)
    {
        printf("SIM cannot spawn player - wrong rules\n");
        return NO;
    }
	//--------------------
	// spawn a player avatar. if this is the first player to spawn,
	// enter gameplay mode
	SimEvent_Spawn* cmd = (SimEvent_Spawn*)sim->data.outputBuf->cursor;
    *cmd = {};
    cmd->header.type = SIM_CMD_TYPE_RESTORE_ENTITY;
    cmd->header.sentinel = ZCMD_SENTINEL;
    cmd->header.size = sizeof(SimEvent_Spawn);
    cmd->factoryType = SIM_TICK_TYPE_ACTOR;
    cmd->pos = sim->info.playerStartPos;
    cmd->serial = Sim_ReserveEntitySerial(sim, NO);
	
    sim->data.outputBuf->cursor += cmd->header.size;

	// set player as in game with avatar Id
	plyr->state = SIM_PLAYER_STATE_IN_GAME;
	plyr->avatarId = cmd->serial;
	
	//--------------------
    // write player state
    ZE_INIT_PTR_IN_PLACE(plyrState, SimEvent_PlayerState, sim->data.outputBuf)
    plyrState->Set(plyr->id, plyr->avatarId, plyr->state);

	// Increment active players and check for game start
	sim->info.numActivePlayers++;
	if (sim->info.numActivePlayers == 1)
	{
		// first player
		printf("SIM - Begin game\n");
        // TODO: Broadcast this
		sim->info.gameState = SIM_GAME_STATE_GAMEPLAY;
	}
    return YES;
}

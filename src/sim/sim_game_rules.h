#include "sim.h"

internal void SimRules_Update(SimScene* sim, timeFloat delta)
{
    if ((sim->flags & SIM_SCENE_BIT_IS_SERVER) == 0) { return; }
    switch (sim->gameRules)
    {
        case SIM_GAME_RULES_SURVIVAL:

        break;
    }
}

internal i32 SimRules_SpawnPlayer(SimScene* sim, SimPlayer* plyr)
{
    if (sim->gameState == SIM_GAME_STATE_GAME_OVER)
	{
		printf("SIM cannot spawn player - game over\n");
		return NO;
	}
    if (sim->gameRules != SIM_GAME_RULES_SURVIVAL)
    {
        printf("SIM cannot spawn player - wrong rules\n");
        return NO;
    }
	// spawn a player avatar. if this is the first player to spawn,
	// enter gameplay mode
	SimEvent_Spawn* cmd = (SimEvent_Spawn*)sim->outputBuf->cursor;
    *cmd = {};
    cmd->header.type = SIM_CMD_TYPE_RESTORE_ENTITY;
    cmd->header.sentinel = ZCMD_SENTINEL;
    cmd->header.size = sizeof(SimEvent_Spawn);
    cmd->factoryType = SIM_TICK_TYPE_ACTOR;
    cmd->pos = sim->playerStartPos;
    cmd->serial = plyr->avatarId;
	
    sim->outputBuf->cursor += cmd->header.size;

	// set player as in game
	plyr->state = SIM_PLAYER_STATE_IN_GAME;

	// Increment active players and check for game start
	sim->numActivePlayers++;
	if (sim->numActivePlayers == 1)
	{
		// first player
		printf("SIM - Begin game\n");
		sim->gameState = SIM_GAME_STATE_GAMEPLAY;
	}
    return YES;
}

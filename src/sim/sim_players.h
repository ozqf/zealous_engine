#ifndef SIM_PLAYERS_H
#define SIM_PLAYERS_H

#include "sim_internal.h"

extern "C" SimPlayer* SimPlyr_Create(SimScene* sim)
{
	SimPlayer* plyr = NULL;
	for (i32 i = 0; i < sim->maxPlayers; ++i)
	{
		if (sim->players[i].state == SIM_PLAYER_STATE_NONE)
		{
			plyr = &sim->players[i];
			plyr->state = SIM_PLAYER_STATE_IN_GAME;
			plyr->id = ++sim->nextPlayerId;
			break;
		}
	}
	return plyr;
}

#endif // SIM_PLAYERS_H
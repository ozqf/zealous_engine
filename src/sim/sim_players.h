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

extern "C" SimPlayer* SimPlyr_Get(SimScene* sim, i32 playerId)
{
	SimPlayer* plyr = NULL;
	for (i32 i = 0; i < sim->maxPlayers; ++i)
	{
		plyr = &sim->players[i];
		if (plyr->state != SIM_PLAYER_STATE_IN_GAME)
		{ continue; }
		if (plyr->id == playerId)
		{ return plyr; }
	}
	return NULL;
}

internal void SimPlyr_Tick(SimScene* sim)
{
	SimPlayer* plyr = NULL;
	for (i32 i = 0; i < sim->maxPlayers; ++i)
	{
		plyr = &sim->players[i];
		if (plyr->state != SIM_PLAYER_STATE_IN_GAME) { continue; }
		if (plyr->avatarId == 0) { continue; }
		SimEntity* ent = Sim_GetEntityBySerial(sim, plyr->avatarId);
		if (ent != NULL)
		{
			ent->input = plyr->input;
		}
		else
		{
			i32 cur = plyr->input.buttons;
			i32 prev = plyr->input.prevButtons;
			if (plyr->input.HasBitToggledOff(ACTOR_INPUT_MOVE_UP))
			{
				// spawn avatar!
				printf("GAME - spawn player %d avatar\n", plyr->id);
			}
		}
		
	}
}

#endif // SIM_PLAYERS_H
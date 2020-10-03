#ifndef SIM_PLAYERS_H
#define SIM_PLAYERS_H

#include "sim_internal.h"

/**
 * Count the number of players who are actively playing
 * and not just connected.
 */
#if 0
internal i32 SimPlyr_InPlayCount(SimScene* sim)
{
	i32 count = 0;
	for (i32 i = 0; i < sim->info.maxPlayers; ++i)
	{
		if (sim->data.players[i].state == SIM_PLAYER_STATE_IN_GAME)
		{
			count++;
		}
	}
	return count;
}
#endif

extern "C" i32 SimPlyr_ReserveId(SimScene* sim)
{
	i32 id = sim->info.nextPlayerId++;
	return id;
}

// if Id is 0 then assign a new Id, otherwise restore reserved
internal SimPlayer* SimPlyr_Create(SimScene* sim, i32 reservedId)
{
	SimPlayer* plyr = NULL;
	for (i32 i = 0; i < sim->info.maxPlayers; ++i)
	{
		if (sim->data.players[i].state == SIM_PLAYER_STATE_NONE)
		{
			plyr = &sim->data.players[i];
			plyr->state = SIM_PLAYER_STATE_OBSERVING;
			if (reservedId == 0)
			{
				plyr->id = ++sim->info.nextPlayerId;
			}
			else
			{
				plyr->id = reservedId;
			}
			break;
		}
	}
	return plyr;
}

extern "C" SimPlayer* SimPlyr_Get(SimScene* sim, i32 playerId)
{
	SimPlayer* plyr = NULL;
	for (i32 i = 0; i < sim->info.maxPlayers; ++i)
	{
		plyr = &sim->data.players[i];
		if (plyr->state == SIM_PLAYER_STATE_NONE)
		{ continue; }
		if (plyr->id == playerId)
		{ return plyr; }
	}
	return NULL;
}

internal void SimPlyr_UpdateState(SimScene* sim, SimEvent_PlayerState* state)
{
	SimPlayer* plyr = SimPlyr_Get(sim, state->playerId);
	if (plyr == NULL)
	{
		printf("SIM no player %d - creating\n", state->playerId);
		plyr = SimPlyr_Create(sim, state->playerId);
	}
	plyr->avatarId = state->avatarId;
	plyr->state = state->state;
	if (state->avatarId != SIM_ENT_NULL_SERIAL)
	{
		SimEntity* ent = Sim_GetEntityBySerial(sim, state->avatarId);
		ZE_ASSERT(ent != NULL, "Player state has avatar Id but no entity!");
		ent->playerId = state->playerId;
	}
}

internal void SimPlyr_Tick(SimScene* sim)
{
	SimPlayer* plyr = NULL;
	for (i32 i = 0; i < sim->info.maxPlayers; ++i)
	{
		plyr = &sim->data.players[i];
		if (plyr->state == SIM_PLAYER_STATE_NONE) { continue; }
		SimEntity* ent = Sim_GetEntityBySerial(sim, plyr->avatarId);

		if (plyr->state == SIM_PLAYER_STATE_OBSERVING)
		{
			ZE_ASSERT(ent == NULL, "Observing player has an avatar...?")
			//printf("Observing player input - %d\n", plyr->input.buttons);
			// read enter game input
			if (plyr->input.HasBitToggledOff(ACTOR_INPUT_MOVE_UP)
				&& SimRules_SpawnPlayer(sim, plyr))
			{
				// spawn avatar!
				printf("GAME - spawn player %d avatar\n", plyr->id);
				//SimPlyr_Spawn(sim, plyr);
			}
		}
		else if (plyr->state == SIM_PLAYER_STATE_IN_GAME)
		{
			ZE_ASSERT(ent != NULL, "Player has no avatar");
			ent->input = plyr->input;
		}
		else
		{
			ZE_ASSERT(NULL, "Unknown player state")
		}
	}
}

internal void SimPlyr_HandleDeath(SimScene* sim, i32 avatarId)
{

}

#endif // SIM_PLAYERS_H
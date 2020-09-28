#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../app.h"
#include "../game_draw/game_draw.h"

#include "game_client.h"
#include "game_server.h"

#define GAME_MAX_MALLOCS 1024
#define GAME_MAX_ENTS 4096

internal MallocItem g_mallocItems[GAME_MAX_MALLOCS];
internal MallocList g_mallocs;
internal SimScene g_sim;
internal frameInt g_systemEventTicks = 0;

internal VWChunk* g_chunk = NULL;

// buffer holds pending commands given to game each tick
internal ZEDoubleBuffer g_gameBuf;

#endif // GAME_INTERNAL_H
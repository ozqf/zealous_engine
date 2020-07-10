#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../app.h"
#include "../client/client_render.h"

#include "game_client.h"
#include "game_server.h"
//#include "game_ent_ticks.h"

#define GAME_MAX_MALLOCS 1024
#define GAME_MAX_ENTS 4096

internal MallocItem g_mallocItems[GAME_MAX_MALLOCS];
internal MallocList g_mallocs;
internal SimScene g_sim;
internal ClientRenderSettings g_rendCfg;
internal ClientRenderer* g_rend;
internal frameInt g_systemEventTicks = 0;

// buffer holds pending commands given to game each tick
internal ZEByteBuffer g_pendingGameInput;

#endif // GAME_INTERNAL_H
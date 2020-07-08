#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../app.h"

#include "game_ent_ticks.h"

#define GAME_MAX_MALLOCS 1024
#define GAME_MAX_ENTS 4096

internal MallocItem g_mallocItems[GAME_MAX_MALLOCS];
internal MallocList g_mallocs;
internal SimScene g_sim;
internal Transform g_camera;

#endif // GAME_INTERNAL_H
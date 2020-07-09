#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "../../ze_common/ze_common_full.h"
#include "../../sim/sim.h"
#include "../app.h"
#include "../client/client_render.h"

#include "game_user_input.h"
#include "game_ent_ticks.h"

#define GAME_MAX_MALLOCS 1024
#define GAME_MAX_ENTS 4096

internal MallocItem g_mallocItems[GAME_MAX_MALLOCS];
internal MallocList g_mallocs;
internal SimScene g_sim;
internal ClientRenderSettings g_rendCfg;
internal ClientRenderer* g_rend;
internal Transform g_camera;
internal SimActorInput g_debugInput;
internal frameInt g_systemEventTicks = 0;

#define GAME_MAX_INPUT_ACTIONS 256
internal InputAction g_inputActionItems[GAME_MAX_INPUT_ACTIONS];
internal InputActionSet g_inputActions = {
    g_inputActionItems,
    0
};

#endif // GAME_INTERNAL_H
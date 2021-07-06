#pragma once

#include "../../headers/common/ze_common.h"

// internal types/function defs
#include "sim_internal.h"

// Implementations
#include "sim_patterns.h"
#include "sim_collision.h"
#include "sim_entity_init.h"
#include "sim_interactions.h"
#include "sim_enemy_defs.h"
#include "sim_projectiles.h"
#include "sim_items.h"
#include "sim_ent_actor.h"
#include "sim_enemy_ticks.h"
#include "sim_entity_ticks.h"
#include "sim_fx.h"
#include "sim_factory.h"
#include "sim_embedded_scenes.h"
#include "sim_fly_camera.h"
#include "sim_players.h"
#include "sim_exec_commands.h"
#include "sim_game_rules.h"
#include "sim_external.h"

// TODO: Uncouple me!
// This should be so that the sim module can access 
// the platform performance counter
//#include "../app_base/app.h"

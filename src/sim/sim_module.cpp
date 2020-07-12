#pragma once

#include "../ze_common/ze_common.h"

#include "sim_internal.h"

#include "sim_patterns.h"
#include "sim_collision.h"
#include "sim_projectiles.h"
#include "sim_entity_init.h"
#include "sim_enemy_defs.h"
#include "sim_items.h"
#include "sim_factory.h"
#include "sim_entity_ticks.h"
#include "sim_embedded_scenes.h"
#include "sim_fly_camera.h"
#include "sim_commands.h"
#include "sim_players.h"
#include "sim_external.h"

// TODO: Uncouple me!
// This should be so that the sim module can access 
// the platform performance counter
//#include "../app_base/app.h"

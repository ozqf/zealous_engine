#include "game_internal.h"

#if 0
#define SVG_DEFINE_ENT_UPDATE(entityTypeName) internal void \
    SVG_Update##entityTypeName##(SimScene* sim, SimEntity* ent, timeFloat deltaTime)

SVG_DEFINE_ENT_UPDATE(LineTrace)
{
    if (sim->tick >= ent->timing.nextThink)
    {
        Sim_RemoveEntity(sim, ent->id.serial);
    }
}


internal void Game_TickEntity(SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
	const i32 bIsServer = YES;
	switch (ent->tickType)
    {
		// case SIM_TICK_TYPE_PROJECTILE:
        // { SVG_UpdateProjectile(sim, ent, deltaTime); } break;
		// case SIM_TICK_TYPE_ACTOR:
        // { SVG_UpdateActor(sim, ent, deltaTime); } break;
        // case SIM_TICK_TYPE_BOT:
        // { SVG_UpdateBot(sim, ent, deltaTime); } break;
        // case SIM_TICK_TYPE_SPAWNER:
        // { SVG_UpdateSpawner(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SEEKER:
        { SimEnt_TickSeeker(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_SEEKER_FLYING:
		{ SimEnt_TickSeekerFlying(sim, ent, deltaTime, bIsServer); } break;
		case SIM_TICK_TYPE_WANDERER:
        { SimEnt_TickWanderer(sim, ent, deltaTime, bIsServer); break; }
        case SIM_TICK_TYPE_BOUNCER:
        { SimEnt_TickBouncer(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_DART:
        { SimEnt_TickDart(sim, ent, deltaTime, bIsServer); } break;
        case SIM_TICK_TYPE_LINE_TRACE:
        { SVG_UpdateLineTrace(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_SPAWN:
        { SimEnt_TickSpawnAnimation(sim, ent, deltaTime); } break;
        case SIM_TICK_TYPE_WORLD: { } break;
        case SIM_TICK_TYPE_NONE: { } break;
        default:
        { ZE_ASSERT(0, "Unknown Ent Tick Type"); } break;
    }
}
#endif
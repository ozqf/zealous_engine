#include "sim.h"

internal Vec3 SimEnt_CalcFlatVelocityTowardPos(
	SimScene* sim, SimEntity* ent, Vec3 target)
{
	Vec3 toTarget =
    {
        target.x - ent->body.t.pos.x,
        0,
        target.z - ent->body.t.pos.z
    };
    Vec3_Normalise(&toTarget);
    if (ent->flags & SIM_ENT_FLAG_MOVE_AVOID)
    {
        SimAvoidInfo avoid = Sim_BuildAvoidVector(sim, ent, 1);
        // Multiply by number of neighbours found
        // to scale move vector up when surrounded
        toTarget.x += avoid.dir.x * avoid.numNeighbours;
        toTarget.y += 0,
        toTarget.z += avoid.dir.z * avoid.numNeighbours;
        Vec3_Normalise(&toTarget);
    }
    return
    {
        toTarget.x * ent->movement.speed,
        0,
        toTarget.z * ent->movement.speed,
    };
}

SIM_DEFINE_ENT_UPDATE_FN(SimEnt_TickProjectileAttack)
{
	switch (ent->think.subMode)
	{
		case 1:
		if (ent->timing.nextThink <= sim->info.tick)
		{
			// fire projectile
			printf("ENT - fire %d\n", sim->info.tick);

			u8 factoryType = SIM_FACTORY_TYPE_PROJECTILE_BASE;
			SimEnt_FireAttack(
				sim, ent, {}, factoryType, 1
			);

			// enter reload pause
			ent->think.subMode = 2;
			ent->timing.nextThink = Sim_CalcThinkTick(sim, 0.5f);
		}
		break;
		case 2:
		if (ent->timing.nextThink <= sim->info.tick)
		{
			printf("ENT - finish attack %d\n", sim->info.tick);
			ent->think.tickType = ent->think.coreTickType;
			ent->think.subMode = SIM_TICK_SUBMODE_NONE;
			ent->timing.nextThink = Sim_CalcThinkTick(sim, 0.5f);
		}
		break;
		default:
		// just entered this state, select think time
		ent->think.subMode = 1;
		printf("ENT - prefire wait %d\n", sim->info.tick);
		// pre-attack wait
		ent->timing.nextThink = Sim_CalcThinkTick(sim, 0.5f);
		break;
	}
}

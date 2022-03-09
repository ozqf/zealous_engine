#include "rng_internal.h"

ze_external i32 ProjectileMask()
{
    return 0;
}

ze_external EntHitResponse Ent_Hit(
	Ent2d* attacker, Ent2d* victim, DamageHit* hit)
{
    // EntityType* attackerType = Sim_GetEntityType(prjEnt->type);
    EntityType* victimType = Sim_GetEntityType(victim->type);
    if (victimType->Hit == NULL)
    {
        return {};
    }
    // RNGPRINT("Hit ent type %s\n", victimType->label);
    EntHitResponse response = victimType->Hit(victim, hit);
    return response;
}

/*
TODO: Ew, poking into the guts of the Spawner type here...
...decide on how to message between entities better.
*/
ze_external void Ent_MessageOnDeathById(i32 messageTargetId, i32 victimId)
{
	Ent2d* ent = Sim_GetEntById(messageTargetId);
	if (ent == NULL) { return; }
	if (ent->type == ENT_TYPE_SPAWNER)
	{
		ent->d.spawner.alive -= 1;
	}
}

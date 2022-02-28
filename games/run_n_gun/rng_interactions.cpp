#include "rng_internal.h"


ze_external EntHitResponse HitEntity(
	Ent2d* attacker, Ent2d* victim, DamageHit* hit)
{
    // EntityType* attackerType = Sim_GetEntityType(prjEnt->type);
    EntityType* victimType = Sim_GetEntityType(victim->type);
    if (victimType->Hit == NULL)
    {
        return {};
    }

    EntHitResponse response = victimType->Hit(victim, hit);
    return response;
}

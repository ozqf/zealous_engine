#include "sim_internal.h"

/**
 * 0 if nothing happened, 1 if damage taken, 2 if victim killed
 */
internal i32 SimEnt_Hit(
	SimScene* sim, SimEntity* attacker, SimEntity* victim, Vec3 hitDir)
{
	if (Sim_IsEntTargetable(victim) == NO) { return 0; }
    ZE_ASSERT(victim->id.serial, "SV overlap victim serial is 0")
    if ((victim->flags & SIM_ENT_FLAG_INVULNERABLE) == 0)
    {
        // Hurt/kill victim
        victim->life.health -= attacker->touchDamage;
        if (victim->life.health <= 0)
        {
            Sim_WriteRemoveEntity(sim, victim, attacker, SIM_DEATH_STYLE_SHOT, hitDir, NO);
			return 2;
        }
		else
		{
			// stun?
			
		}
		
    }
	return 1;
}

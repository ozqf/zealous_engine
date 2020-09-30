#include "sim_internal.h"

internal i32 SimEnt_CheckTeamDiffer(i32 atkTeam, i32 victimTeam)
{
    if (victimTeam == SIM_ENT_TEAM_FREELANCE) { return YES; }
    if (victimTeam == SIM_ENT_TEAM_NON_COMBATANT) { return NO; }
    return atkTeam != victimTeam;
}

/**
 * 0 if nothing happened, 1 if damage taken, 2 if victim killed
 */
internal i32 SimEnt_Hit(
	SimScene* sim, SimEntity* attacker, SimEntity* victim, Vec3 hitDir)
{
    // projectiles cannot hit their own parents!
    if (attacker->relationships.parentId.serial == victim->id.serial)
    {
        return 0;
    }
    // check eligability to hit
	if (Sim_IsEntTargetable(victim) == NO) { return 0; }
    if (!SimEnt_CheckTeamDiffer(attacker->teamId, victim->teamId)) { return 0; }

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
			victim->think.tickType = SIM_TICK_TYPE_STUN;
            victim->timing.nextThink = Sim_CalcThinkTick(
                sim, victim->life.stunDuration);
            // Launch off the ground
			if (victim->movement.moveMode == SIM_ENT_MOVE_TYPE_WALK)
			{
                Vec3 launchVel;
                if (victim->movement.flags & SIM_ENT_MOVE_BIT_GROUNDED)
                { launchVel = { 0, 10, 0 }; }
                else { launchVel = { 0, 2, 0 }; }
                if (victim->movement.velocity.y < launchVel.y)
                {
                    victim->movement.velocity = launchVel;
                }
			}
		}
    }
	return 1;
}

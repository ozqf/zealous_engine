#pragma once

#include "sim_internal.h"

internal void Sim_InitProjectile(
    SimEntity* ent,
    Vec3 pos,
    Vec3 eulerDegrees,
    Vec3 velocity,
    SimProjectileType* type,
    i32 fastForwardTicks)
{
    ent->status = SIM_ENT_STATUS_IN_USE;
	ent->tickType = SIM_TICK_TYPE_PROJECTILE;
	ent->coreTickType = SIM_TICK_TYPE_PROJECTILE;
    ent->flags |= SIM_ENT_FLAG_USE_OVERRIDE_SCALE;
    ent->display.scale = { 0.15f, 0.15f, 1.0f };
	Transform_SetToIdentity(&ent->body.t);
    Transform_SetRotation(&ent->body.t, eulerDegrees.x * DEG2RAD, eulerDegrees.y * DEG2RAD, 0);
    // check scale
    Vec3* v = &ent->body.t.scale;
    if (v->x <= 0) { v->x = 1; }
    if (v->y <= 0) { v->y = 1; }
    if (v->z <= 0) { v->z = 1; }
    
	ent->body.t.pos = pos;
    ent->body.previousPos = pos;
    ent->body.t.scale = ent->display.scale;
    ent->movement.velocity = velocity;
    ent->movement.speed = type->speed;

    ent->timing.nextThink = ent->timing.birthTick +
        App_CalcTickInterval(type->lifeTime);
	ent->timing.fastForwardTicks = fastForwardTicks;
}
internal i32 Sim_StepProjectile(
    SimScene* sim, SimEntity* ent, timeFloat deltaTime)
{
    Vec3 frameOrigin = ent->body.t.pos;
    Vec3 frameMove;
    frameMove.x = ent->movement.velocity.x * (f32)deltaTime;
    frameMove.y = ent->movement.velocity.y * (f32)deltaTime;
    frameMove.z = ent->movement.velocity.z * (f32)deltaTime;
    Vec3 frameDest;
    frameDest.x = frameOrigin.x + frameMove.x;
    frameDest.y = frameOrigin.y + frameMove.y;
    frameDest.z = frameOrigin.z + frameMove.z;
    // get direction for hits
    Vec3 dir = frameMove;
    Vec3_Normalise(&dir);
    // by raycast
    const i32 max_overlaps = 16;
    SimRaycastResult results[max_overlaps];
    i32 overlaps = 0;
    overlaps = Sim_FindByRaycast(
        sim, frameOrigin, frameDest, {}, ent->id.serial, results, max_overlaps);
    i32 killed = NO;
    //printf("SIM prj hits %d\n", overlaps);
    for (i32 i = 0; i < overlaps; ++i)
    {
        SimEntity* victim = results[i].ent;
        if (Sim_IsEntTargetable(victim) == NO) { continue; }
        ZE_ASSERT(victim->id.serial, "SV overlap victim serial is 0")
        
        if ((victim->flags & SIM_ENT_FLAG_INVULNERABLE) == 0)
        {
            // Hurt/kill victim
            victim->life.health -= ent->touchDamage;
            if (victim->life.health <= 0)
            {
                Sim_WriteRemoveEntity(sim, victim, ent, SIM_DEATH_STYLE_SHOT, dir, NO);
            }
        }
        frameDest = results[i].hitPos;
        killed = YES;

        break;
    }
    
    // place projectile at end of move
    ent->body.t.pos = frameDest;
    
    if (killed == YES)
    {
        /*printf("PRJ real birth tick vs sim tick: %d, %d\n", (i32)ent->timing.realBirthTick, (i32)sim->tick);
        if (ent->timing.realBirthTick == sim->tick)
        {
            printf("\tPRJ died on birth tick!\n");
        }*/
        Sim_WriteRemoveEntity(sim, ent, NULL, SIM_DEATH_STYLE_SHOT, Vec3_Flipped(dir), NO);
        return 0;
    }
    
    // Timeout
	if (sim->tick >= ent->timing.nextThink)
	{
        Sim_WriteRemoveEntity(sim, ent, NULL, SIM_DEATH_STYLE_TIMEOUT, Vec3_Flipped(dir), YES);
        return 0;
	}
    return 1;
}

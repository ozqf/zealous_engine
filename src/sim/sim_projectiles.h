#pragma once

#include "sim_internal.h"

internal void Sim_InitProjectile(
    SimEntity* ent,
    Vec3* pos,
    Vec3* velocity,
    SimProjectileType* type,
    i32 fastForwardTicks)
{
    ent->status = SIM_ENT_STATUS_IN_USE;
	ent->tickType = SIM_TICK_TYPE_PROJECTILE;
	ent->coreTickType = SIM_TICK_TYPE_PROJECTILE;
    ent->flags |= SIM_ENT_FLAG_USE_OVERRIDE_SCALE;
    ent->display.scale = { 0.15f, 0.15f, 1.0f };
	Transform_SetToIdentity(&ent->body.t);
    ent->body.t.scale = type->scale;
    // check scale
    Vec3* v = &ent->body.t.scale;
    if (v->x <= 0) { v->x = 1; }
    if (v->y <= 0) { v->y = 1; }
    if (v->z <= 0) { v->z = 1; }
    
	ent->body.t.pos = *pos;
    ent->movement.velocity = *velocity;
    ent->movement.speed = type->speed;

    ent->timing.nextThink = ent->timing.birthTick +
        App_CalcTickInterval(type->lifeTime);
	ent->timing.fastForwardTicks = fastForwardTicks;
}

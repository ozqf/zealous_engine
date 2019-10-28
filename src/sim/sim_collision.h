#pragma once

#include "sim_internal.h"
#include "../ze_common/ze_collision.h"

/**
 * Returns number of overlapping entities
 */
extern "C"
inline i32 Sim_FindByAABB(
    SimScene* sim,
    Vec3 boundsMin,
    Vec3 boundsMax,
    i32 ignoreSerial,
    SimEntity** results,
    i32 maxResults,
	i32 replicatedOnly
    )
{
	//AppTimer timer(APP_STAT_AABB_SEARCH, g_apptick++);
	i64 start = App_SampleClock();
    i32 resultIndex = 0;
    i32 count = 0;
	
	// Cut down search range - should not try to avoid local entities
	// as the client cannot recreate this.
	i32 len = replicatedOnly == NO ? sim->maxEnts : len = (sim->maxEnts / 2);
	
	// TODO: This is super inefficient. Need a proper spatial partition
    for (i32 i = 0; i < len; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE
            || ent->id.serial == ignoreSerial)
        { continue; }
        
        if (Sim_IsEntInPlay(ent) == NO) { continue; }

        // expand bounds by entity size and
        // then point test
        Vec3 halfSize = 
        {
            ent->body.t.scale.x,
            ent->body.t.scale.y,
            ent->body.t.scale.z
        };
		
		#if 1
        Vec3 min;
        min.x = boundsMin.x - halfSize.x;
        min.y = boundsMin.y - halfSize.y;
        min.z = boundsMin.z - halfSize.z;
        Vec3 max;
        max.x = boundsMax.x + halfSize.x;
        max.y = boundsMax.y + halfSize.y;
        max.z = boundsMax.z + halfSize.z;
        Vec3* p = &ent->body.t.pos;
        if (p->x < min.x || p->x > max.x) { continue; }
        if (p->y < min.y || p->y > max.y) { continue; }
        if (p->z < min.z || p->z > max.z) { continue; }
		
		if (p->x < min.x || p->x > max.x) { continue; }
        if (p->y < min.y || p->y > max.y) { continue; }
        if (p->z < min.z || p->z > max.z) { continue; }
		#endif
        count++;
        if (results)
        {
            results[resultIndex++] = ent;
            if (count >= maxResults)
            {
                break;
            }
        }
    }
	i64 end = App_SampleClock();
	sim->timeInAABBSearch += (end - start);
    return count;
}

/**
 * Returns number of overlapping entities
 */
extern "C"
i32 Sim_FindByRaycast(
    SimScene* sim,
    Vec3 origin,
    Vec3 dest,
    i32 ignoreSerial,
    SimEntity** results,
    i32 maxResults
    )
{
    i32 resultIndex = 0;
    i32 count = 0;
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (ent->status != SIM_ENT_STATUS_IN_USE
            || ent->id.serial == ignoreSerial)
        { continue; }
        if (ent->tickType != SIM_TICK_TYPE_WANDERER)
        { continue; }
        // create aabb for ent and line test
        Vec3* p = &ent->body.t.pos;
        Vec3 min;
        min.x = p->x - 0.5f;
        min.y = p->y - 0.5f;
        min.z = p->z - 0.5f;
        Vec3 max;
        max.x = p->x + 0.5f;
        max.y = p->y + 0.5f;
        max.z = p->z + 0.5f;
        
        u8 hit = LineSegmentVsAABB(
            origin.x, origin.y, origin.z,
            dest.x, dest.y, dest.z,
            min.x, min.y, min.z,
            min.x, min.y, min.z,
            NULL
        );
        if (!hit) { continue; }
        count++;
        if (results)
        {
            results[resultIndex] = ent;
            if (count >= maxResults)
            {
                break;
            }
        }
    }
    return count;
}

extern "C"
SimAvoidInfo Sim_BuildAvoidVector(
    SimScene* sim,
    SimEntity* mover)
{
    SimAvoidInfo info = {};
    #if 0
    return info;
    #endif
    // TODO: Optimise
    #if 1
    Vec3 halfSize =
    {
        // Actually, don't divide by 2. Inflat volume
        // to include more objects, keeping things
        // further apart
        mover->body.t.scale.x,// / 2,
        mover->body.t.scale.y,// / 2,
        mover->body.t.scale.z// / 2
    };
    Vec3 p = mover->body.t.pos;
    Vec3 min;
    min.x = p.x - halfSize.x;
    min.y = p.y - halfSize.y;
    min.z = p.z - halfSize.z;
    Vec3 max;
    max.x = p.x + halfSize.x;
    max.y = p.y + halfSize.y;
    max.z = p.z + halfSize.z;

    const i32 maxResults = 32;
    SimEntity* results[maxResults];
    i32 numOverlaps = Sim_FindByAABB(
		sim, min, max, mover->id.serial, results, maxResults, YES);
    if (numOverlaps == 0) { return { }; }
    Vec3 origin = mover->body.t.pos;
    for (i32 i = 0; i < numOverlaps; ++i)
    {
        SimEntity* ent = results[i];
        if (!(ent->flags & SIM_ENT_FLAG_MOVE_AVOID)) { continue; }
        Vec3 other = ent->body.t.pos;
        Vec3 diff =
        {
            origin.x - other.x,
            origin.y - other.y,
            origin.z - other.z
        };
        Vec3_Normalise(&diff);
        info.dir.x += diff.x;
        info.dir.y += diff.y;
        info.dir.z += diff.z;
        info.numNeighbours++;
    }
    return info;
    #endif
}

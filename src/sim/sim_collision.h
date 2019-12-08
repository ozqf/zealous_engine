#ifndef SIM_COLLISION_H
#define SIM_COLLISION_H

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
	timeFloat start = App_SampleClock();
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
	timeFloat end = App_SampleClock();
	sim->timeInAABBSearch += (end - start);
    return count;
}

extern "C" i32 Sim_FindClosestRayhit(SimRaycastResult* results, i32 numResults)
{
    //printf("Find closest: ");
    i32 resultIndex = -1;
    f32 resultDist = Z_INFINITY;
    for (i32 i = 0; i < numResults; ++i)
    {
        SimRaycastResult* item = &results[i];
        if (item->dist < resultDist)
        {
            //printf("%.3f < %.3f - ", item->dist, resultDist);
            resultIndex = i;
            resultDist = item->dist;
        }
    }
    //printf("\n");
    return resultIndex;
}

/**
 * Returns number of overlapping entities
 */
extern "C"
i32 Sim_FindByRaycast(
    SimScene* sim,
    Vec3 origin,
    Vec3 dest,
    Vec3 objSizeInflate,
    i32 ignoreSerial,
    SimRaycastResult* results,
    i32 maxResults
    )
{
    i32 count = 0;
    for (i32 i = 0; i < sim->maxEnts; ++i)
    {
        SimEntity* ent = &sim->ents[i];
        if (Sim_IsEntTargetable(ent) == NO) { continue; }
        if (ent->id.serial == ignoreSerial) { continue; }
        // create aabb for ent and line test
        Vec3* p = &ent->body.t.pos;
        Vec3* size = &ent->body.t.scale;
        Vec3 halfSize;
        halfSize.x = (size->x + objSizeInflate.x) / 2;
        halfSize.y = (size->y + objSizeInflate.y) / 2;
        halfSize.z = (size->z + objSizeInflate.z) / 2;
        Vec3 min;
        min.x = p->x - halfSize.x;
        min.y = p->y - halfSize.y;
        min.z = p->z - halfSize.z;
        Vec3 max;
        max.x = p->x + halfSize.x;
        max.y = p->y + halfSize.y;
        max.z = p->z + halfSize.z;
        Vec3 hitPos;
        f32 hitFraction;
        u8 hit = LineSegmentVsAABB(
            origin.x, origin.y, origin.z,
            dest.x, dest.y, dest.z,
            min.x, min.y, min.z,
            max.x, max.y, max.z,
            hitPos.parts,
            &hitFraction
        );
        if (!hit) { continue; }

        if (results)
        {
            SimRaycastResult* result = &results[count];
            result->fraction = hitFraction;
            result->hitPos = hitPos;
            result->normal = { 0, 1, 0 };
            result->ent = ent;
            result->dist = Vec3_Distance(origin, hitPos);

            if (count >= maxResults) { break; }
        }

        count++;
    }
    return count;
}

static void SimEnt_MoveVsSolid(SimScene* sim, SimEntity* ent, Vec3 move)
{
    Vec3 origin = ent->body.t.pos;
    Vec3 dest = origin;
    dest.x += move.x;
    dest.y += move.y;
    dest.z += move.z;

    //Vec3 entSize = 
    const i32 max_overlaps = 16;
    SimRaycastResult results[max_overlaps];
    i32 overlaps = Sim_FindByRaycast(
        sim, origin, dest, ent->body.t.scale, ent->id.serial, results, max_overlaps);
    if (overlaps > 0)
    {
        i32 closestIndex = Sim_FindClosestRayhit(results, overlaps);
        //dest = results[closestIndex].hitPos;
        printf("Actor overlap fraction %.3f\n", results[closestIndex].fraction);
    }
    ent->body.t.pos = dest;

}

extern "C"
SimAvoidInfo Sim_BuildAvoidVector(
    SimScene* sim,
    SimEntity* mover,
    f32 boundingBoxScale)
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
        (mover->body.t.scale.x / 2) * boundingBoxScale,
        (mover->body.t.scale.y / 2) * boundingBoxScale,
        (mover->body.t.scale.z / 2) * boundingBoxScale
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

#endif // SIM_COLLISION_H
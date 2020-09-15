#pragma once

/**
 * Iterate over a set of entity links, calculating their
 * priority to the given avatar
 */
#include "../../sim/sim.h"
#include "../shared/priority_queue.h"

internal void SVP_CalculatePriorities(
    SimScene* sim, SimEntity* subject, PriorityLink* links, i32 numLinks)
{
    Vec3 pos = subject->body.t.pos;
    f32 closest = Z_INFINITY, furthest = 0;
    // Scan over links, finding nearest and furthest
    // Do in reverse so links can be removed
    for (i32 i = numLinks - 1; i >= 0; --i)
    {
        PriorityLink* link = &links[i];
        if (link->status != ENT_LINK_STATUS_ACTIVE)
        {
            // Force to maximum for death updates
            link->distance = 0;
            continue;
        }

        SimEntity* ent = Sim_GetEntityBySerial(sim, link->id);
        ZE_ASSERT(ent, "Linked entity is null")
        Vec3 entPos = ent->body.t.pos;
        f32 dist = Vec3_Distance(pos, entPos);
        link->distance = dist;
        if (dist > furthest) { furthest = dist; }
        if (dist < closest) { closest = dist; }
    }

    // Find each link's place in the group
    for (i32 i = numLinks - 1; i >= 0; --i)
    {
        PriorityLink* link = &links[i];
        
        // Calculate range based priority by comparing to distance
        // of other entities.
        f32 dist = link->distance;
		
		f32 furthestZeroBased = furthest - closest;
		f32 distZeroBased = dist - closest;
		f32 multiplier = distZeroBased / furthestZeroBased;
		
		link->priority = (u8)((SIM_NET_MAX_PRIORITY - 1) * (1 - multiplier));
		link->priority += 1;

        // This is just for debugging yeah?
        if (link->status == ENT_LINK_STATUS_ACTIVE)
        {
            SimEntity* ent = Sim_GetEntityBySerial(sim, link->id);
            ZE_ASSERT(ent != NULL, "Linked entity is null");
            ent->priority = link->priority;
        }
    }
}

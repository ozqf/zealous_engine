#include "sim_internal.h"

internal void SimGfx_EntityDeath(SimScene* sim, SimEvent_RemoveEnt* cmd)
{
	SimEntity* ent = Sim_GetEntityBySerial(sim, cmd->entityId);
	if (ent == NULL) { return; }

	switch (ent->deathType)
    {
        case SIM_DEATH_GFX_BULLET_IMPACT:
        {
            SimEvent_Spawn def = {};
            def.factoryType = SIM_FACTORY_TYPE_BULLET_IMPACT;
            def.birthTick = sim->tick;
            def.pos = ent->body.t.pos;
            def.serial = Sim_ReserveEntitySerial(sim, 1);
            Sim_RestoreEntity(sim, &def);
            // Test particle spray
            Vec3 pos = def.pos;
            for (i32 i = 0; i < 5; ++i)
            {
                f32 rand = COM_STDRandf32();
                Vec3 vel;
                vel.x = COM_STDRandomInRange(-15, 15);
                vel.y = COM_STDRandomInRange(-10, 15);
                vel.z = COM_STDRandomInRange(-15, 15);
                //CLR_SpawnTestParticle(g_rend, CLR_PARTICLE_TYPE_TEST, pos, vel);
            }
            
        } break;
        case SIM_DEATH_GFX_GIB:
        {
            // Test particle spray
            Vec3 pos = ent->body.t.pos;
            for (i32 i = 0; i < 5; ++i)
            {
                f32 rand = COM_STDRandf32();
                Vec3 vel;
                vel.x = COM_STDRandomInRange(-10, 10);
                vel.y = COM_STDRandomInRange(-5, 20);
                vel.z = COM_STDRandomInRange(-10, 10);
                //CLR_SpawnTestParticle(g_rend, CLR_PARTICLE_TYPE_GIB, pos, vel);
            }
        } break;
    }
}

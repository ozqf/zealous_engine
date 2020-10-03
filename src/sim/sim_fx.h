#include "sim_internal.h"

internal void SimFx_EntityDeath(SimScene* sim, SimEvent_RemoveEnt* cmd)
{
	SimEntity* ent = Sim_GetEntityBySerial(sim, cmd->entityId);
	if (ent == NULL) { return; }

	switch (ent->deathType)
    {
        case SIM_DEATH_GFX_BULLET_IMPACT:
        {
            // spawn local impact blob
            SimEvent_Spawn def = {};
            def.factoryType = SIM_FACTORY_TYPE_BULLET_IMPACT;
            def.birthTick = sim->info.tick;
            def.pos = ent->body.t.pos;
            def.serial = Sim_ReserveEntitySerial(sim, YES);
            Sim_RestoreEntity(sim, &def);

            // write particle spray
            ZCMD_STRUCT_IN_PLACE(
                SimEvent_Particles, ev, SIM_CMD_TYPE_PARTICLES, sim->data.outputBuf)
            ev->particleEventType = SIM_DEATH_GFX_BULLET_IMPACT;
            ev->pos = ent->body.t.pos;
            
            // write sound
            ZE_INIT_PTR_IN_PLACE(soundEv, ZSoundCommand, sim->data.soundOutputBuf);
            if (soundEv != NULL)
            {
                //printf("SIM - init sound event\n");
                soundEv->type = ZSOUND_EVENT_PLAY;
                soundEv->data.play.soundEventType = 1;
                soundEv->data.play.pos = ent->body.t.pos;
            }
            else
            {
                printf("SIM - no space for sound event (%d bytes left)\n", sim->data.soundOutputBuf->Space());
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

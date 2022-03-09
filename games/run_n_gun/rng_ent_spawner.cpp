#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

// EntSpawner
// SpawnerSave

ze_internal EntSpawner* GetSpawner(Ent2d* ent)
{
	return (EntSpawner*)&ent->d.spawner;
}

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	SpawnerSave* save = (SpawnerSave*)stateHeader;
	Ent2d* ent = Sim_GetEntById(save->header.id);
	EntSpawner* spawner = NULL;
	if (ent != NULL)
	{
		spawner = GetSpawner(ent);
		*spawner = save->data;
	}
	else
	{
		ent = Sim_GetFreeEntity(save->header.id, ENT_TYPE_SPAWNER);
		spawner = GetSpawner(ent);
		*spawner = save->data;
	}

	// mark ent with latest restore tick
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void Write(Ent2d* ent, ZEBuffer* buf)
{
	EntSpawner* spawner = GetSpawner(ent);
	ZE_BUF_INIT_PTR_IN_PLACE(save, SpawnerSave, buf);

	save->header = Ent_SaveHeaderFromEnt(ent, sizeof(SpawnerSave));
	save->data = *spawner;
}

ze_internal void Remove(Ent2d* ent){}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	EntSpawner* spawner = GetSpawner(ent);
	if (spawner->alive >= spawner->maxAlive)
	{
		return;
	}

	if (spawner->tick <= 0.f)
	{
		spawner->tick = spawner->delay;
		// RNGPRINT("Spawner - tick!\n");
		Sim_SpawnEnemyGrunt(spawner->pos, ent->id);
		spawner->alive += 1;
	}
	else
	{
		spawner->tick -= delta;
	}
}

ze_internal void Sync(Ent2d* ent){}
ze_internal EntHitResponse Hit(Ent2d* victim, DamageHit* hit) { return {}; }
ze_internal void Print(Ent2d* ent){}

ze_external void Sim_SpawnSpawner(Vec2 pos)
{
	SpawnerSave save = {};
	save.header = Ent_SaveHeaderFromRaw(
		Sim_ReserveDynamicIds(1), 0, ENT_TYPE_SPAWNER, sizeof(SpawnerSave));
	save.data.alive = 0;
	save.data.delay = 3.f;
	save.data.maxAlive = 3;
	save.data.pos = pos;
	save.data.spawnType = ENT_TYPE_ENEMY_GRUNT;
	save.data.tick = 0;
	save.data.totalSpawns = 10;

	Sim_RestoreEntity(&save.header);
}

ze_external void EntSpawner_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();
	type->type = ENT_TYPE_SPAWNER;
	type->label = "My New Type Name";
	type->Restore = Restore;
	type->Write = Write;
	type->Remove = Remove;
	type->Tick = Tick;
	type->Sync = Sync;
	type->Hit = Hit;
	type->Print = Print;
}

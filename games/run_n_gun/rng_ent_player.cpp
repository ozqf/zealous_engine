#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	PlayerEntState* state = (PlayerEntState*)stateHeader;
	Ent2d* ent = Sim_GetEntById(state->header.id);
	if (ent != NULL)
	{
		// restore
	}
	else
	{
		// Create
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(state->header.id);
		ent->type = ENT_TYPE_PLAYER;
		ent->id = state->header.id;
		
	}
	
	ent->lastRestoreTick = restoreTick;
	ent->tick = state->tick;
}

ze_internal void Write(Ent2d* ent, ZEBuffer* buf)
{
	
}

ze_internal void Remove(Ent2d* ent)
{
	
}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	
}

ze_external void EntPlayer_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();
	type->type = ENT_TYPE_PLAYER;
	type->label = "Player";
	type->Restore = Restore;
	type->Write = Write;
	type->Remove = Remove;
	type->Tick = Tick;
}

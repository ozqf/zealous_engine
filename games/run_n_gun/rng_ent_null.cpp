/*
Empty base entity type code.
Shows basic blueprint concrete entities extend
*/
#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	// ...cast header to concrete state here...
	
	// retrieve entity. if not found, add, otherwise restore
	Ent2d* ent = Sim_GetEntById(stateHeader->id);
	if (ent != NULL)
	{
		// ...restore concrete data...
	}
	else
	{
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(stateHeader->id);
		ent->type = ENT_TYPE_DEBRIS;
		ent->id = stateHeader->id;

		// ...restore concrete data...
	}

	// mark ent with latest restore tick
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void Write(Ent2d* ent, ZEBuffer* buf)
{
	EntStateHeader* state = (EntStateHeader*)buf->cursor;
	buf->cursor += sizeof(EntStateHeader);
	
	state->type = ent->type;
	state->id = ent->id;
	state->numBytes = sizeof(EntStateHeader);
}

ze_internal void Remove(Ent2d* ent)
{
	// concrete cleanup code here...

	// ...final base cleanup
	ent->type = ENT_TYPE_NONE;
	Sim_RemoveEntityBase(ent);
}

ze_internal void Tick(Ent2d* ent, f32 delta) { }
ze_internal void Sync(Ent2d* ent) { }

ze_external void EntNull_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();
	type->type = ENT_TYPE_NONE;
	type->label = "null";
	type->Restore = Restore;
	type->Write = Write;
	type->Remove = Remove;
	type->Tick = Tick;
	type->Sync = Sync;
}
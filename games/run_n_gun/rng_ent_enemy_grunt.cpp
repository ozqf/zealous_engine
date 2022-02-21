/*
Basic ground based enemy with simple projectile attack.
*/
#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	// ...cast header to concrete state here...
	EntGruntSave* state = (EntGruntSave*)stateHeader;
	
	// retrieve entity. if not found, add, otherwise restore
	Ent2d* ent = Sim_GetEntById(stateHeader->id);
	EntGrunt* grunt = NULL;
	if (ent != NULL)
	{
		// ...restore concrete data...
	}
	else
	{
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(stateHeader->id);
		ent->type = ENT_TYPE_ENEMY_GRUNT;
		ent->id = stateHeader->id;

		grunt = &ent->d.grunt;

		// ...restore concrete data...
		
		// add physics body
		ZPBodyDef def = {};
		def.bIsStatic = NO;
		def.bLockRotation = YES;
		def.friction = 0;
		def.resitition = 0;
		def.shape.radius = { 0.5f, 0.5f, };
		def.shape.pos = state->pos;
		
		grunt->physicsBodyId = ZP_AddBody(def);
		
		// state
		grunt->aimDegrees = state->aimDegrees;
		grunt->targetId = state->targetId;
		grunt->tick = state->tick;

		// add sprites
		ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, "enemy");
		ZGen_FillTexture(tex, COLOUR_U32_RED);
		ZRDrawObj* sprite = g_engine.scenes.AddFullTextureQuad(
			g_scene, "enemy", {0.5f, 0.5f});
		sprite->t.pos.z = state->depth;
		
		ZRDrawObj* weapon = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_NAME, {0.7f, 0.1f});
		
		grunt->bodyDrawId = sprite->id;
		grunt->gunDrawId = weapon->id;
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

ze_internal void Tick(Ent2d* ent, f32 delta)
{

}

ze_internal void Sync(Ent2d* ent)
{
	
}

ze_external void EntGrunt_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();
	type->type = ENT_TYPE_ENEMY_GRUNT;
	type->label = "null";
	type->Restore = Restore;
	type->Write = Write;
	type->Remove = Remove;
	type->Tick = Tick;
	type->Sync = Sync;
}

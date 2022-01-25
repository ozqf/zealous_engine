#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void RestoreDebris(EntStateHeader* stateHeader, u32 restoreTick)
{
	DebrisEntState* state = (DebrisEntState*)stateHeader;
	Ent2d* ent = Sim_GetEntById(state->header.id);
	if (ent != NULL)
	{
		// restore state
		BodyState body = {};
		body.t.pos = state->pos;
		body.t.radians = state->degrees * DEG2RAD;
		body.velocity = state->velocity;
		body.angularVelocity = state->angularVelocity;
		ZP_SetBodyState(ent->bodyId, body);
	}
	else
	{
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(state->header.id);
		ent->type = ENT_TYPE_DEBRIS;
		ent->id = state->header.id;

		// add sprite
		ZRDrawObj *debris = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
		ent->drawId = debris->id;
		debris->t.pos = Vec3_FromVec2(state->pos, state->depth);

		// add body
		ZPBodyDef def = {};
		def.bIsStatic = NO;
		def.bLockRotation = NO;
		def.friction = 0.5f;
		def.resitition = 0.5f;
		def.shape.radius = { 0.5f, 0.5f };
		
		// restore state
		def.shape.pos = state->pos;
		
		ent->bodyId = ZP_AddBody(def);
	}

	// mark ent with latest restore tick
	ent->lastRestoreTick = restoreTick;
	ent->tick = state->tick;
}

ze_internal void WriteDebris(Ent2d* ent, ZEBuffer* buf)
{
	DebrisEntState* state = (DebrisEntState*)buf->cursor;
	buf->cursor += sizeof(DebrisEntState);
	
	state->header.type = ent->type;
	state->header.id = ent->id;
	state->header.numBytes = sizeof(DebrisEntState);
	
	BodyState body = ZP_GetBodyState(ent->bodyId);
	
	state->pos = body.t.pos;
	state->degrees = body.t.radians * RAD2DEG;
	state->velocity = body.velocity;
	state->angularVelocity = body.angularVelocity;
}

ze_internal void Remove(Ent2d* ent)
{
	// printf("Remove debris\n");
	Sim_RemoveEntityBase(ent);
}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	ent->tick += delta;
	if (ent->tick > 5.f)
	{
		// printf("Tick end - debris\n");
		Sim_RemoveEntity(ent);
	}
	else
	{
		// printf("Tick debris\n");
	}
}

ze_external void EntDebris_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();
	type->type = ENT_TYPE_DEBRIS;
	type->label = "Debris";
	type->Restore = RestoreDebris;
	type->Write = WriteDebris;
	type->Remove = Remove;
	type->Tick = Tick;
}

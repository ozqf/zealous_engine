#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void RestoreDebris(EntStateHeader* stateHeader, u32 restoreTick)
{
	DebrisEntSave* state = (DebrisEntSave*)stateHeader;
	Ent2d* ent = Sim_GetEntById(state->header.id);
	if (ent != NULL)
	{
		// restore state
		BodyState body = {};
		body.t.pos = state->pos;
		body.t.radians = state->degrees * DEG2RAD;
		body.velocity = state->velocity;
		body.angularVelocity = state->angularVelocity;
		ZP_SetBodyState(ent->d.debris.physicsBodyId, body);

		ent->d.debris.tick = state->tick;
	}
	else
	{
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(state->header.id, ENT_TYPE_DEBRIS);

		// add sprite
		ZRDrawObj *debris = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
		ent->d.debris.drawId = debris->id;
		debris->t.pos = Vec3_FromVec2(state->pos, state->depth);

		// add body
		ZPBodyDef def = {};
		def.bIsStatic = NO;
		def.bLockRotation = NO;
		def.friction = 0.5f;
		def.resitition = 0.5f;
		def.shape.radius = { 0.5f, 0.5f };
		def.externalId = ent->id;
		
		// restore state
		def.shape.pos = state->pos;
		ent->d.debris.tick = state->tick;
		
		ent->d.debris.physicsBodyId = ZP_AddBody(def);
	}

	// mark ent with latest restore tick
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void WriteDebris(Ent2d* ent, ZEBuffer* buf)
{
	DebrisEntSave* state = (DebrisEntSave*)buf->cursor;
	buf->cursor += sizeof(DebrisEntSave);
	
	state->header.type = ent->type;
	state->header.id = ent->id;
	state->header.numBytes = sizeof(DebrisEntSave);
	
	state->tick = ent->d.debris.tick;
	
	BodyState body = ZP_GetBodyState(ent->d.debris.physicsBodyId);
	
	state->pos = body.t.pos;
	state->degrees = body.t.radians * RAD2DEG;
	state->velocity = body.velocity;
	state->angularVelocity = body.angularVelocity;
}

ze_internal void Remove(Ent2d* ent)
{
	// printf("Remove debris\n");
	g_engine.scenes.RemoveObject(g_scene, ent->d.debris.drawId);
	ZP_RemoveBody(ent->d.debris.physicsBodyId);
	ent->type = ENT_TYPE_NONE;
	Sim_RemoveEntityBase(ent);
}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	ent->d.debris.tick += delta;
	if (ent->d.debris.tick > 5.f)
	{
		// printf("Tick end - debris\n");
		Sim_RemoveEntity(ent);
	}
	else
	{
		// printf("Tick debris\n");
	}
}

ze_internal void Sync(Ent2d* ent)
{
	EntDebris* debris = &ent->d.debris;
	Sim_SyncDrawObjToPhysicsObj(debris->drawId, debris->physicsBodyId);
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
	type->Sync = Sync;
}

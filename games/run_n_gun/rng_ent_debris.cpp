#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

#define DEBRIS_LIFE_TIME 10.f

ze_internal void RestoreBodyState(DebrisEntSave* state, zeHandle physicsBodyId)
{
	// restore state
	BodyState body = {};
	body.t.pos = state->pos;
	body.t.radians = state->degrees * DEG2RAD;
	body.velocity = state->velocity;
	body.angularVelocity = state->angularVelocity;
	ZP_SetBodyState(physicsBodyId, body);
}

ze_internal void RestoreDebris(EntStateHeader* stateHeader, u32 restoreTick)
{
	DebrisEntSave* state = (DebrisEntSave*)stateHeader;
	Ent2d* ent = Sim_GetEntById(state->header.id);
	if (ent != NULL)
	{
		RestoreBodyState(state, ent->d.debris.physicsBodyId);
		ent->d.debris.tick = state->tick;
	}
	else
	{
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(state->header.id, ENT_TYPE_DEBRIS);

		// add sprite
		ZRDrawObj *debris = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_WHITE, {0.5f, 0.5f}, COLOUR_F32_PURPLE);
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
		def.categoryBits = PHYSICS_LAYER_BIT_DEBRIS;
		def.maskBits = PHYSICS_LAYER_BIT_DEBRIS | PHYSICS_LAYER_BIT_WORLD;
		
		// restore state
		def.shape.pos = state->pos;
		ent->d.debris.tick = state->tick;
		
		ent->d.debris.physicsBodyId = ZP_AddBody(def);
		RestoreBodyState(state, ent->d.debris.physicsBodyId);
	}

	// mark ent with latest restore tick
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void WriteDebris(Ent2d* ent, ZEBuffer* buf)
{
	DebrisEntSave* state = (DebrisEntSave*)buf->cursor;
	buf->cursor += sizeof(DebrisEntSave);
	
	state->header = Ent_SaveHeaderFromEnt(ent, sizeof(DebrisEntSave));
	
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

ze_internal void Tick(Ent2d* ent, RNGTickInfo* tickInfo)
{
	if (ent->d.debris.tick >= DEBRIS_LIFE_TIME)
	{
		Sim_RemoveEntity(ent);
	}
	else
	{
		ent->d.debris.tick += tickInfo->delta;
	}
}

ze_internal void Sync(Ent2d* ent)
{
	EntDebris* debris = &ent->d.debris;
	Sim_SyncDrawObjToPhysicsObj(debris->drawId, debris->physicsBodyId);

	ColourF32 full = { 0.5f, 0.f, 0.f, 1.f };
	ColourF32 fade = { 0.1f, 0.1f, 0.1f, 1.f };
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, debris->drawId);
	f32 weight = (debris->tick / DEBRIS_LIFE_TIME);
	obj->data.quad.colour = ColourF32Lerp(full, fade, weight);
}

ze_external EntHitResponse DebrisHit(Ent2d* victim, DamageHit* hit)
{
	// TODO: Push debris a bit from hit.
	// RNGPRINT("Hit debris\n");
	Vec2 force = Vec2_Mul(hit->dir, 100.f);
	// ZP_ApplyForce(victim->d.debris.physicsBodyId, force);
	ZP_ApplyForceAtPoint(victim->d.debris.physicsBodyId, force, hit->pos);
	EntHitResponse response = {};
	response.responseType = ENT_HIT_RESPONSE_NONE;
	return response;
}

ze_external void Sim_SpawnDebris(Vec2 pos, Vec2 velocity, f32 spin)
{
	// RNGPRINT("Spawning debris at %.3f, %.3f\n", pos.x, pos.y);
	DebrisEntSave debris = {};
	debris.header = Ent_SaveHeaderFromRaw(
		Sim_ReserveDynamicIds(1),
		ENT_EMPTY_TAG,
		ENT_TYPE_DEBRIS,
		sizeof(DebrisEntSave)
	);
	debris.pos = pos;
	debris.depth = -0.1f;
	debris.velocity = velocity;
	debris.angularVelocity = spin;
	Sim_GetEntityType(ENT_TYPE_DEBRIS)->Restore(&debris.header, Sim_GetRestoreTick());
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
	type->Hit = DebrisHit;
}

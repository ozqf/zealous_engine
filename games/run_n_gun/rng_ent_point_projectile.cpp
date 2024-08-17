/*
Most basic projectile type. moves as a ray every tick
*/
#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal EntPointProjectile* GetPointPrj(Ent2d* ent)
{
	return &ent->d.pointPrj;
}

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	EntPointProjectileSave* state = (EntPointProjectileSave*)stateHeader;
	Ent2d* ent = Sim_GetEntById(state->header.id);
	EntPointProjectile* prj = NULL;
	if (ent != NULL)
	{
		prj = &ent->d.pointPrj;
		// restore concrete data
		ent->d.pointPrj.data = state->data;
	}
	else
	{
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(state->header.id, ENT_TYPE_POINT_PRJ);

		prj = &ent->d.pointPrj;
		// restore concrete data
		prj->data = state->data;
		
		// add sprite
		ZRDrawObj* sprite = NULL;
		if (ent->d.pointPrj.data.teamId == TEAM_ID_PLAYER)
		{
			sprite = g_engine.scenes.AddFullTextureQuad(
				g_scene, FALLBACK_TEXTURE_WHITE, {0.25f, 0.25f}, COLOUR_F32_YELLOW);
		}
		else
		{
			sprite = g_engine.scenes.AddFullTextureQuad(
				g_scene, FALLBACK_TEXTURE_WHITE, {0.25f, 0.25f}, COLOUR_F32_RED);
		}
		ent->d.pointPrj.comp.drawId = sprite->id;
		sprite->t.pos = { prj->data.pos.x, prj->data.pos.y, prj->data.depth };
	}

	// mark ent with latest restore tick
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void Write(Ent2d* ent, ZEBuffer* buf)
{
	EntPointProjectileSave* state = (EntPointProjectileSave*)buf->cursor;
	buf->cursor += sizeof(EntPointProjectileSave);
	
	state->header = Ent_SaveHeaderFromEnt(ent, sizeof(EntPointProjectileSave));
	state->data = ent->d.pointPrj.data;
}

ze_internal void Remove(Ent2d* ent)
{
	// concrete cleanup code here...
	g_engine.scenes.RemoveObject(g_scene, ent->d.pointPrj.comp.drawId);

	// ...final base cleanup
	ent->type = ENT_TYPE_NONE;
	Sim_RemoveEntityBase(ent);
}

// return yes if hit should destroy the prjectile
ze_internal i32 HitEnt(Ent2d* self, i32 entId, i32 volumeId, DamageHit* dmg)
{
	Ent2d* victim = Sim_GetEntById(entId);
	if (victim == NULL)
	{
		if (volumeId != 0)
		{
			BodyState hitBody = ZP_GetBodyState(volumeId);
			RNGPRINT("PRJ hit volume %d with no ent. External Id %d\n",
				volumeId, entId);
		}
		Sim_SpawnGfx(dmg->pos, 0);
		return YES;
	}
	EntHitResponse response = Ent_Hit(self, victim, dmg);
	//RNGPRINT("PRJ on team %d hit ent %d: response %d\n",
	//	prj->data.teamId, victim->id, response.responseType);
	if (response.responseType != ENT_HIT_RESPONSE_NONE)
	{
		Sim_SpawnGfx(dmg->pos, 0);
		return YES;
	}
	return NO;
}

ze_internal void Tick(Ent2d* ent, RNGTickInfo* tickInfo)
{
	EntPointProjectile* prj = GetPointPrj(ent);
	prj->data.tick += tickInfo->delta;
	if (prj->data.tick > 10.0)
	{
		//RNGPRINT("prj timeout\n");
		Remove(ent);
	}
	Vec2 dir;
	Vec2 move;
	f32 speed = 50.f;
	switch (prj->data.templateId)
	{
		case PRJ_TEMPLATE_ENEMY_DEFAULT:
		speed = 7.5f;
		break;
	}

	dir.x = cosf(prj->data.radians);
	dir.y = sinf(prj->data.radians);
	move = Vec2_Mul(dir, speed);
	// move.x = cosf(prj->data.radians) * speed;
	// move.y = sinf(prj->data.radians) * speed;
	Vec2 dest = Vec2_Add(prj->data.pos, Vec2_Mul(move, tickInfo->delta));

	
	// prepare data if we need to hit something
	DamageHit dmg = {};
	dmg.damage = 10;
	dmg.teamId = prj->data.teamId;
	dmg.normal = Vec2_Mul(dir, -1);
	dmg.dir = dir;

	// raycasts ignore any shapes they start in, so do a point test first to find anything that
	// moved into our position during the last physics step
	const i32 maxResults = 16;
	i32 numResults = 0;
	i32 bCull = NO;
	u16 mask = PHYSICS_LAYER_BIT_WORLD | PHYSICS_LAYER_BIT_MOBS | PHYSICS_LAYER_BIT_PLAYER_HITBOX | PHYSICS_LAYER_BIT_DEBRIS;
	
	ZAABBResult aabbs[maxResults];
	Vec2 min, max;
	min.x = prj->data.pos.x - 0.05f;
	min.y = prj->data.pos.y - 0.05f;
	max.x = prj->data.pos.x + 0.05f;
	max.y = prj->data.pos.y + 0.05f;
	numResults = ZP_AABBCast(min, max, aabbs, maxResults, mask);
	if (numResults > 0)
	{
		ZAABBResult* hit = &aabbs[numResults - 1];
		dmg.pos = prj->data.pos;
		bCull = HitEnt(ent, hit->externalId, hit->volumeId, &dmg);
		if (bCull)
		{
			Remove(ent);
			return;
		}
	}
	
	ZPRaycastResult results[maxResults];
	numResults = ZP_Raycast(prj->data.pos, dest, results, maxResults, mask);
	if (numResults > 0)
	{
		ZPRaycastResult* hit = &results[numResults - 1];
		dmg.pos = hit->pos;
		bCull = HitEnt(ent, hit->externalId, hit->volumeId, &dmg);
	}
	if (bCull)
	{
		Remove(ent);
		return;
	}
	else
	{
		prj->data.pos = dest;
	}
}

ze_internal void Sync(Ent2d* ent)
{
	EntPointProjectile* prj = GetPointPrj(ent);
	ZRDrawObj* sprite = g_engine.scenes.GetObject(g_scene, prj->comp.drawId);
	Vec3 pos = Vec3_FromVec2(prj->data.pos, prj->data.depth);
	sprite->t.pos = pos;
}

ze_external void Sim_SpawnProjectile(Vec2 pos, f32 degrees, i32 teamId, i32 templateId)
{
	EntPointProjectileSave prj = {};
	prj.header = Ent_SaveHeaderFromRaw(
		Sim_ReserveDynamicIds(1),
		ENT_EMPTY_TAG,
		ENT_TYPE_POINT_PRJ,
		sizeof(EntPointProjectileSave)
	);
	
	// apply projectile template
	prj.data.templateId = templateId;

	// instance info
	prj.data.pos = pos;
	prj.data.depth = 0.1f;
	prj.data.teamId = teamId;
	prj.data.radians = degrees * DEG2RAD;
	Sim_GetEntityType(ENT_TYPE_POINT_PRJ)->Restore(&prj.header, Sim_GetRestoreTick());
}

ze_external void EntPointProjectile_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();
	type->type = ENT_TYPE_NONE;
	type->label = "PointProjectile";
	type->Restore = Restore;
	type->Write = Write;
	type->Remove = Remove;
	type->Tick = Tick;
	type->Sync = Sync;
}

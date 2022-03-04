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
				g_scene, POINTPRJ_PLAYER_TEX, {0.25f, 0.25f});
		}
		else
		{
			sprite = g_engine.scenes.AddFullTextureQuad(
				g_scene, POINTPRJ_ENEMY_TEX, {0.25f, 0.25f});
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
	
	state->header.type = ent->type;
	state->header.id = ent->id;
	state->header.numBytes = sizeof(EntPointProjectileSave);

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

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	EntPointProjectile* prj = GetPointPrj(ent);
	prj->data.tick += delta;
	if (prj->data.tick > 2.0)
	{
		//RNGPRINT("prj timeout\n");
		Remove(ent);
	}
	Vec2 move;
	f32 speed = 30.f;
	switch (prj->data.templateId)
	{
		case PRJ_TEMPLATE_ENEMY_DEFAULT:
		speed = 7.5f;
		break;
	}
	move.x = cosf(prj->data.radians) * speed;
	move.y = sinf(prj->data.radians) * speed;
	Vec2 dest = Vec2_Add(prj->data.pos, Vec2_Mul(move, delta));
	ZPRaycastResult results[16];
	i32 bCull = NO;
	i32 numResults = ZP_Raycast(prj->data.pos, dest, results, 16);
	if (numResults > 0)
	{
		ZPRaycastResult* hit = &results[numResults - 1];
		// printf("Prj raycast got %d result, hit volumeId %d (external %d)\n",
		// 	numResults,
		// 	hit->volumeId,
		// 	hit->externalId);
		Ent2d* victim = Sim_GetEntById(hit->externalId);
		if (victim == NULL)
		{
			BodyState hitBody = ZP_GetBodyState(hit->volumeId);
			RNGPRINT("PRJ on team %d hit volume %d with no ent. External Id %d\n",
				prj->data.teamId, hit->volumeId, hitBody.externalId);
			bCull = YES;
		}
		else
		{
			DamageHit dmg = {};
			dmg.damage = 10;
			dmg.teamId = prj->data.teamId;
			dmg.pos = hit->pos;
			dmg.normal = hit->normal;
			EntHitResponse response = HitEntity(ent, victim, &dmg);
			//RNGPRINT("PRJ on team %d hit ent %d: response %d\n",
			//	prj->data.teamId, victim->id, response.responseType);
			if (response.responseType != ENT_HIT_RESPONSE_NONE)
			{
				bCull = YES;
			}
		}
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

	ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, POINTPRJ_PLAYER_TEX);
		ZGen_FillTexture(tex, COLOUR_U32_YELLOW);
	
	tex = g_engine.assets.AllocTexture(16, 16, POINTPRJ_ENEMY_TEX);
		ZGen_FillTexture(tex, COLOUR_U32_RED);
}

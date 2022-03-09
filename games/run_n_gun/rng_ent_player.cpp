#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal EntPlayer* GetPlayer(Ent2d* ent)
{
	return &ent->d.player;
}

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	PlayerEntSave* state = (PlayerEntSave*)stateHeader;
	Ent2d* ent = Sim_GetEntById(state->header.id);
	if (ent != NULL)
	{
		// restore
		// RNGPRINT("Restore player entId %d\n", ent->id);

		// physics
		BodyState body = {};
		body.t.pos = state->pos;
		body.velocity = state->velocity;
		ZP_SetBodyState(ent->d.player.physicsBodyId, body);

		// other
		ent->d.player.aimDegrees = state->aimDegrees;
		ent->d.player.buttons = state->buttons;
		ent->d.player.tick = state->tick;
		ent->d.player.status = state->status;
	}
	else
	{
		// Create
		// RNGPRINT("Create player entId %d\n", state->header.id);

		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(state->header.id, ENT_TYPE_PLAYER);
		
		// add physics body
		ZPBodyDef def = {};
		def.bIsStatic = NO;
		def.bLockRotation = YES;
		def.friction = 0;
		def.resitition = 0;
		def.shape.radius = { 0.5f, 0.5f, };
		def.externalId = ent->id;
		
		// restore state
		def.shape.pos = state->pos;
		ent->d.player.tick = state->tick;
		ent->d.player.status = state->status;
		
		ent->d.player.physicsBodyId = ZP_AddBody(def);
		RNGPRINT("Player body Id: %d\n", ent->d.player.physicsBodyId);
		
		// add sprites
		ZRDrawObj* sprite = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_WHITE, {0.5f, 0.5f}, COLOUR_F32_GREEN);

		printf("Player sprite colour: %.3f, %.3f, %.3f\n",
			sprite->data.quad.colour.r,
			sprite->data.quad.colour.g,
			sprite->data.quad.colour.b);
		
		ZRDrawObj* weapon = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_WHITE, {0.7f, 0.1f}, COLOUR_F32_LIGHT_GREY);
		
		ent->d.player.bodyDrawId = sprite->id;
		ent->d.player.gunDrawId = weapon->id;
	}
	
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void Write(Ent2d* ent, ZEBuffer* buf)
{
	PlayerEntSave* state = (PlayerEntSave*)buf->cursor;
	buf->cursor += sizeof(PlayerEntSave);

	// header
	state->header = Ent_SaveHeaderFromEnt(ent, sizeof(PlayerEntSave));

	// data
	state->tick = ent->d.player.tick;
	state->aimDegrees = ent->d.player.aimDegrees;
	state->buttons = ent->d.player.buttons;
	state->status = ent->d.player.status;

	// components
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, ent->d.player.bodyDrawId);
	state->depth = obj->t.pos.z;
	
	BodyState body = ZP_GetBodyState(ent->d.player.physicsBodyId);
	state->pos = body.t.pos;
	state->velocity = body.velocity;
}

ze_internal void Remove(Ent2d* ent)
{
	g_engine.scenes.RemoveObject(g_scene, ent->d.player.bodyDrawId);
	g_engine.scenes.RemoveObject(g_scene, ent->d.player.gunDrawId);
	
	ZP_RemoveBody(ent->d.player.physicsBodyId);
	
	ent->type = ENT_TYPE_NONE;
	Sim_RemoveEntityBase(ent);
}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	EntPlayer* player = GetPlayer(ent);
	player->tick -= delta;
	// input
	/*
	INPUT_BIT_LEFT
	INPUT_BIT_RIGHT
	INPUT_BIT_UP
	INPUT_BIT_DOWN
	INPUT_BIT_ATK_1
	INPUT_BIT_ATK_2
	INPUT_BIT_USE
	*/
	Vec2 inputDir = {};
	if (IF_BIT(player->buttons, INPUT_BIT_LEFT)) { inputDir.x--; }
	if (IF_BIT(player->buttons, INPUT_BIT_RIGHT)) { inputDir.x++; }
	if (IF_BIT(player->buttons, INPUT_BIT_DOWN)) { inputDir.y--; }
	if (IF_BIT(player->buttons, INPUT_BIT_UP)) { inputDir.y++; }
	
	BodyState body = ZP_GetBodyState(player->physicsBodyId);
	Vec2 v = body.velocity;
	v.x = 8.f * inputDir.x;

	if (inputDir.y > 0)
	{
		v.y = 8.f;
	}
	else if (inputDir.y < 0 && v.y > 1.0f)
	{
		v.y = -2;
	}

	ZP_SetLinearVelocity(player->physicsBodyId, v);

	// update aim pos
	Vec2 pos = body.t.pos;
	Vec2 aimPos = Sim_GetTickInfo()->cursorWorldPos;
	f32 radians = Vec2_AngleTo(pos, aimPos);
	player->aimDegrees = radians * RAD2DEG;

	if (IF_BIT(player->buttons, INPUT_BIT_ATK_1))
	{
		if (player->tick <= 0)
		{
			// shoot
			player->tick = 0.1f;
			Sim_SpawnProjectile(
				pos, player->aimDegrees, TEAM_ID_PLAYER, PRJ_TEMPLATE_PLAYER_DEFAULT);
		}
	}
	else if (IF_BIT(player->buttons, INPUT_BIT_ATK_2))
	{
		if (player->tick <= 0)
		{
			// shoot
			player->tick = 0.5f;
			Sim_SpawnProjectile(
				pos, player->aimDegrees, TEAM_ID_PLAYER, PRJ_TEMPLATE_PLAYER_DEFAULT);
			Sim_SpawnProjectile(
				pos, player->aimDegrees - 7.5f, TEAM_ID_PLAYER, PRJ_TEMPLATE_PLAYER_DEFAULT);
			Sim_SpawnProjectile(
				pos, player->aimDegrees - 15.f, TEAM_ID_PLAYER, PRJ_TEMPLATE_PLAYER_DEFAULT);
			Sim_SpawnProjectile(
				pos, player->aimDegrees + 7.5f, TEAM_ID_PLAYER, PRJ_TEMPLATE_PLAYER_DEFAULT);
			Sim_SpawnProjectile(
				pos, player->aimDegrees + 15.f, TEAM_ID_PLAYER, PRJ_TEMPLATE_PLAYER_DEFAULT);
		}
	}
}

ze_internal void Sync(Ent2d* ent)
{
	EntPlayer* player = GetPlayer(ent);
	Sim_SyncDrawObjToPhysicsObj(player->bodyDrawId, player->physicsBodyId);
	
	BodyState body = ZP_GetBodyState(player->physicsBodyId);
	
	// point gun
	f32 radians = player->aimDegrees * DEG2RAD;
	Vec3 pos = Vec3_FromVec2(body.t.pos, 0);
	pos.x += cosf(radians) * 0.5f;
	pos.y += sinf(radians) * 0.5f;
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, player->gunDrawId);
	obj->t.pos = pos;
	M3x3_SetToIdentity(obj->t.rotation.cells);
	M3x3_RotateZ(obj->t.rotation.cells, radians);
	
	// camera
	#if 0
	Transform camera = g_engine.scenes.GetCamera(g_scene);
	camera.pos.x = body.t.pos.x;
	camera.pos.y = body.t.pos.y;
	g_engine.scenes.SetCamera(g_scene, camera);
	#endif
}

ze_internal EntHitResponse PlayerHit(Ent2d* victim, DamageHit* hit)
{
	EntHitResponse response = {};
	if (hit->teamId == TEAM_ID_PLAYER)
	{
		response.responseType = ENT_HIT_RESPONSE_NONE;
	}
	else
	{
		// RNGPRINT("Player hit by %d!\n", hit->teamId);
		response.responseType = ENT_HIT_RESPONSE_DAMAGED;
		victim->d.player.status = PLAYER_STATUS_DEAD;
	}
	return response;
}

ze_external void EntPlayer_SetInput(RNGTickInfo info)
{
	Ent2d* ent = Sim_GetEntById(ENT_RESERVED_ID_PLAYER);
	if (ent == NULL || ent->type != ENT_TYPE_PLAYER)
	{
		return;
	}
	ent->d.player.buttons = info.buttons;
	
	// calculate aim degrees
	BodyState body = ZP_GetBodyState(ent->d.player.physicsBodyId);
	Vec2 pos = body.t.pos;
	Vec2 toCursor = Vec2_Subtract(info.cursorWorldPos, pos);
	ent->d.player.aimDegrees = atan2f(toCursor.y, toCursor.x) * RAD2DEG;
	// info.cursorWorldPos
	// ent->d.player.aimDegrees = info.aimDegrees;
}

ze_external void Sim_SpawnPlayer(Vec2 pos)
{
	// pos.x = 0;
	// pos.y = 5;
	RNGPRINT("Spawn player at %.3f, %.3f\n", pos.x, pos.y);
	PlayerEntSave player = {};
	player.header = Ent_SaveHeaderFromRaw(
		ENT_RESERVED_ID_PLAYER,
		ENT_EMPTY_TAG,
		ENT_TYPE_PLAYER,
		sizeof(PlayerEntSave)
	);
	
	player.pos = pos;
	player.status = PLAYER_STATUS_PLAYING;
	Sim_GetEntityType(ENT_TYPE_PLAYER)->Restore(&player.header, Sim_GetRestoreTick());
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
	type->Sync = Sync;
	type->Hit = PlayerHit;
}

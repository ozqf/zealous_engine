/*
Basic ground based enemy with simple projectile attack.
*/
#include "rng_internal.h"

ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;

ze_internal void Restore(EntStateHeader* stateHeader, u32 restoreTick)
{
	// ...cast header to concrete save here...
	EntGruntSave* save = (EntGruntSave*)stateHeader;
	
	// retrieve entity. if not found, add, otherwise restore
	Ent2d* ent = Sim_GetEntById(stateHeader->id);
	EntGrunt* grunt = NULL;
	if (ent != NULL)
	{
		// ...restore concrete data...
		grunt = &ent->d.grunt;

		// physics
		BodyState body = {};
		body.t.pos = save->pos;
		body.velocity = save->velocity;
		//RNGPRINT("Restore grunt body to %.3f, %.3f\n", body.t.pos.x, body.t.pos.y);
		ZP_SetBodyState(grunt->physicsBodyId, body);

		// other
		grunt->aimDegrees = save->aimDegrees;
		grunt->targetId = save->targetId;
		grunt->tick = save->tick;
		grunt->health = save->health;
	}
	else
	{
		// add entity and restore core entity info
		ent = Sim_GetFreeEntity(stateHeader->id, ENT_TYPE_ENEMY_GRUNT);

		grunt = &ent->d.grunt;

		// ...restore concrete data...
		
		// add physics body
		ZPBodyDef def = {};
		def.bIsStatic = NO;
		def.bLockRotation = YES;
		def.friction = 0;
		def.resitition = 0;
		def.shape.radius = { 0.5f, 0.5f, };
		def.shape.pos = save->pos;
		def.externalId = ent->id;
		
		//RNGPRINT("Create grunt body at %.3f, %.3f\n", def.shape.pos.x, def.shape.pos.y);
		grunt->physicsBodyId = ZP_AddBody(def);
		
		// data
		grunt->aimDegrees = save->aimDegrees;
		grunt->targetId = save->targetId;
		grunt->tick = save->tick;
		grunt->health = save->health;

		// add sprites
		ZRDrawObj* sprite = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_WHITE, {0.5f, 0.5f}, COLOUR_F32_RED);
		
		sprite->t.pos.z = save->depth;
		
		ZRDrawObj* weapon = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_WHITE, {0.7f, 0.1f}, COLOUR_F32_WHITE);
		
		grunt->bodyDrawId = sprite->id;
		grunt->gunDrawId = weapon->id;

		BodyState bodyState = ZP_GetBodyState(grunt->physicsBodyId);
		RNGPRINT("Created fresh grunt. entId %d, bodyId %d, externalId %d\n",
			ent->id, grunt->physicsBodyId, bodyState.externalId);
	}

	// mark ent with latest restore tick
	ent->lastRestoreFrame = restoreTick;
}

ze_internal void Write(Ent2d* ent, ZEBuffer* buf)
{
	EntGrunt* grunt = &ent->d.grunt;
	EntGruntSave* save = (EntGruntSave*)buf->cursor;
	buf->cursor += sizeof(EntGruntSave);

	// header
	save->header.type = ent->type;
	save->header.id = ent->id;
	save->header.numBytes = sizeof(EntGruntSave);

	// logic
	save->aimDegrees = grunt->aimDegrees;
	save->tick = grunt->tick;
	save->targetId = grunt->targetId;
	save->health = grunt->health;
	
	// components
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, grunt->bodyDrawId);
	save->depth = obj->t.pos.z;
	
	BodyState body = ZP_GetBodyState(grunt->physicsBodyId);
	save->pos = body.t.pos;
	save->velocity = body.velocity;
}

ze_internal void Remove(Ent2d* ent)
{
	// concrete cleanup code here...
	g_engine.scenes.RemoveObject(g_scene, ent->d.grunt.bodyDrawId);
	g_engine.scenes.RemoveObject(g_scene, ent->d.grunt.gunDrawId);
	
	ZP_RemoveBody(ent->d.grunt.physicsBodyId);
	
	// ...final base cleanup
	ent->type = ENT_TYPE_NONE;
	Sim_RemoveEntityBase(ent);
}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	EntGrunt* grunt = &ent->d.grunt;
	Ent2d* playerEnt = Sim_FindPlayer();
	
	BodyState body = ZP_GetBodyState(grunt->physicsBodyId);

	// tick attacking
	if (playerEnt != NULL)
	{
		// look at player
		Vec2 pos = body.t.pos;
		BodyState playerBody = ZP_GetBodyState(playerEnt->d.player.physicsBodyId);
		Vec2 aimPos = playerBody.t.pos;
		grunt->aimDegrees = Vec2_AngleTo(pos, aimPos) * RAD2DEG;
		if (grunt->tick <= 0)
		{
			grunt->tick = 0.5f;
			Sim_SpawnProjectile(
				pos, grunt->aimDegrees, TEAM_ID_ENEMY, PRJ_TEMPLATE_ENEMY_DEFAULT);
		}
		else
		{
			grunt->tick -= delta;
		}
	}
	else
	{
		grunt->aimDegrees = 0;
	}
}

ze_internal void Sync(Ent2d* ent)
{
	EntGrunt* grunt = (EntGrunt*)&ent->d.grunt;
	Sim_SyncDrawObjToPhysicsObj(grunt->bodyDrawId, grunt->physicsBodyId);


	// point gun
	f32 radians = grunt->aimDegrees * DEG2RAD;
	BodyState body = ZP_GetBodyState(grunt->physicsBodyId);
	Vec3 pos = Vec3_FromVec2(body.t.pos, 0);

	pos.x += cosf(radians) * 0.5f;
	pos.y += sinf(radians) * 0.5f;

	ZRDrawObj* obj = g_engine.scenes.GetObject(g_scene, grunt->gunDrawId);
	obj->t.pos = pos;
	M3x3_SetToIdentity(obj->t.rotation.cells);
	M3x3_RotateZ(obj->t.rotation.cells, radians);
}

ze_external EntHitResponse GruntHit(Ent2d* victim, DamageHit* hit)
{
	EntHitResponse response = {};
	if (hit->teamId == TEAM_ID_ENEMY)
	{
		response.responseType = ENT_HIT_RESPONSE_NONE;
	}
	else
	{
		EntGrunt* grunt = &victim->d.grunt;
		grunt->health -= hit->damage;
		if (grunt->health <= 0)
		{
			response.damageDone = hit->damage + grunt->health;
			response.responseType = ENT_HIT_RESPONSE_KILLED;
			RNGPRINT("Grunt %d, bodyId %d dying. Normal %.3f, %.3f\n",
				victim->id, grunt->physicsBodyId, hit->normal.x, hit->normal.y);
			Transform2d bodyT = ZP_GetBodyPosition(grunt->physicsBodyId);
			Vec2 vel = Vec2_Mul(Vec2_Mul(hit->normal, -1.f), 6.f);
			// kick off the ground regardless of direction of hit
			vel.y += 5.f;
			Sim_SpawnDebris(bodyT.pos, vel, 0.f);
			Remove(victim);
		}
		else
		{
			response.damageDone = hit->damage;
			response.responseType = ENT_HIT_RESPONSE_DAMAGED;
		}
	}
	return response;
}

ze_internal void GruntPrint(Ent2d* ent)
{
	RNGPRINT("BodyId %d ", ent->d.grunt.physicsBodyId);
}

ze_external void Sim_SpawnEnemyGrunt(Vec2 pos)
{
	// pos.x = 0;
	// pos.y = 5;
	//RNGPRINT("Spawn player at %.3f, %.3f\n", pos.x, pos.y);
	EntGruntSave grunt = {};
	grunt.header.type = ENT_TYPE_ENEMY_GRUNT;
	grunt.header.numBytes = sizeof(EntGruntSave);
	grunt.header.id = Sim_ReserveDynamicIds(1);
	grunt.pos = pos;
	grunt.health = 70;
	grunt.state = 0;
	Sim_GetEntityType(ENT_TYPE_ENEMY_GRUNT)->Restore(&grunt.header, Sim_GetRestoreTick());
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
	type->Hit = GruntHit;
	type->Print = GruntPrint;
}

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
		//RNGPRINT("Create grunt body at %.3f, %.3f\n", def.shape.pos.x, def.shape.pos.y);
		grunt->physicsBodyId = ZP_AddBody(def);
		
		// data
		grunt->aimDegrees = save->aimDegrees;
		grunt->targetId = save->targetId;
		grunt->tick = save->tick;

		// add sprites
		ZRDrawObj* sprite = g_engine.scenes.AddFullTextureQuad(
			g_scene, TEX_ENEMY, {0.5f, 0.5f});
		sprite->t.pos.z = save->depth;
		
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
			Sim_SpawnProjectile(pos, grunt->aimDegrees, TEAM_ID_ENEMY);
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
	Sim_GetEntityType(ENT_TYPE_ENEMY_GRUNT)->Restore(&grunt.header, Sim_GetRestoreTick());
}

ze_external void EntGrunt_Register(EntityType* type)
{
	g_engine = GetEngine();
	g_scene = GetGameScene();

	ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, TEX_ENEMY);
	ZGen_FillTexture(tex, COLOUR_U32_RED);

	type->type = ENT_TYPE_ENEMY_GRUNT;
	type->label = "null";
	type->Restore = Restore;
	type->Write = Write;
	type->Remove = Remove;
	type->Tick = Tick;
	type->Sync = Sync;
}

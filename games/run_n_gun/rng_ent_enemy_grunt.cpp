/*
Basic ground based enemy with simple projectile attack.
*/
#include "rng_internal.h"

#define GRUNT_THINK_TIME 2.f
#define GRUNT_WALK_SPEED 4.f

#define AI_STATE_IDLE 0
#define AI_STATE_STUNNED 1

#define OVERLAP_LEFT (1 << 0)
#define OVERLAP_RIGHT (1 << 1)
#define OVERLAP_UP (1 << 2)
#define OVERLAP_DOWN (1 << 3)
#define OVERLAP_FLOOR_LEFT (1 << 4)
#define OVERLAP_FLOOR_RIGHT (1 << 5)

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
		grunt->state = save->state;
		grunt->aimDegrees = save->aimDegrees;
		grunt->targetId = save->targetId;
		grunt->tick = save->tick;
		grunt->tock = save->tock;
		grunt->health = save->health;
		grunt->sourceId = save->sourceId;
		grunt->dirX = save->dirX;
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
		def.categoryBits = PHYSICS_LAYER_BIT_MOBS;
		def.maskBits = PHYSICS_LAYER_BIT_WORLD | PHYSICS_LAYER_BIT_PLATFORM;
		
		//RNGPRINT("Create grunt body at %.3f, %.3f\n", def.shape.pos.x, def.shape.pos.y);
		grunt->physicsBodyId = ZP_AddBody(def);
		
		// data
		grunt->state = save->state;
		grunt->aimDegrees = save->aimDegrees;
		grunt->targetId = save->targetId;
		grunt->tick = save->tick;
		grunt->tock = save->tock;
		grunt->health = save->health;
		grunt->sourceId = save->sourceId;
		grunt->dirX = 1;

		// add sprites
		ZRDrawObj* sprite = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_WHITE, {0.5f, 0.5f}, COLOUR_F32_RED);
		
		sprite->t.pos.z = save->depth;
		
		ZRDrawObj* weapon = g_engine.scenes.AddFullTextureQuad(
			g_scene, FALLBACK_TEXTURE_WHITE, {0.7f, 0.1f}, COLOUR_F32_WHITE);
		
		grunt->bodyDrawId = sprite->id;
		grunt->gunDrawId = weapon->id;

		BodyState bodyState = ZP_GetBodyState(grunt->physicsBodyId);
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
	save->header = Ent_SaveHeaderFromEnt(ent, sizeof(EntGruntSave));
	
	// logic
	save->state = grunt->state;
	save->aimDegrees = grunt->aimDegrees;
	save->tick = grunt->tick;
	save->targetId = grunt->targetId;
	save->health = grunt->health;
	save->sourceId = grunt->sourceId;
	save->dirX = grunt->dirX;
	
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

const Vec2 g_scanLeftOffsetMin = { -0.6f, -0.1f };
const Vec2 g_scanLeftOffsetMax = { -0.4f, 0.1f };

const Vec2 g_scanRightOffsetMin = { 0.4f, -0.1f };
const Vec2 g_scanRightOffsetMax = { 0.6f, 0.1f };

const Vec2 g_scanFloorLeftOffsetMin = { -0.6f, -0.6f };
const Vec2 g_scanFloorLeftOffsetMax = { -0.4f, -0.4f };

const Vec2 g_scanFloorRightOffsetMin = { 0.4f, -0.6f };
const Vec2 g_scanFloorRightOffsetMax = { 0.6f, -0.4f };

ze_internal i32 ScanWithOffset(Vec2 pos, Vec2 minOffset, Vec2 maxOffset)
{
	Vec2 min = pos, max = pos;
	min.x += minOffset.x;
	min.y += minOffset.y;
	max.x += maxOffset.x;
	max.y += maxOffset.y;

	i32 flags = 0;
	u16 mask = PHYSICS_LAYER_BIT_WORLD | PHYSICS_LAYER_BIT_PLATFORM;
	const i32 maxResults = 16;
	ZAABBResult results[maxResults];
	i32 numHits = ZP_AABBCast(min, max, results, maxResults, mask);
	return numHits > 0;
}

ze_internal i32 RefreshSensorFlags(BodyState body)
{
	i32 flags = 0;
	Vec2 pos = body.t.pos;
	if (ScanWithOffset(pos, g_scanLeftOffsetMin, g_scanLeftOffsetMax))
	{
		flags |= OVERLAP_LEFT;
	}
	if (ScanWithOffset(pos, g_scanRightOffsetMin, g_scanRightOffsetMax))
	{
		flags |= OVERLAP_RIGHT;
	}
	if (ScanWithOffset(pos, g_scanFloorLeftOffsetMin, g_scanFloorLeftOffsetMax))
	{
		flags |= OVERLAP_FLOOR_LEFT;
	}
	if (ScanWithOffset(pos, g_scanFloorRightOffsetMin, g_scanFloorRightOffsetMax))
	{
		flags |= OVERLAP_FLOOR_RIGHT;
	}
	return flags;
}

ze_internal void WalkTick(EntGrunt* grunt, f32 delta)
{
	BodyState body = ZP_GetBodyState(grunt->physicsBodyId);
	Vec2 vel = body.velocity;
	i32 flags = RefreshSensorFlags(body);
	if (grunt->dirX == 1)
	{
		// moving right
		if (!IF_BIT(flags, OVERLAP_RIGHT) && IF_BIT(flags, OVERLAP_FLOOR_RIGHT))
		{
			vel.x = GRUNT_WALK_SPEED;
		}
		else
		{
			grunt->dirX = -1;
		}
	}
	else
	{
		if (!IF_BIT(flags, OVERLAP_LEFT) && IF_BIT(flags, OVERLAP_FLOOR_LEFT))
		{
			vel.x = -GRUNT_WALK_SPEED;
		}
		else
		{
			grunt->dirX = 1;
		}
		// moving left
	}
	ZP_SetLinearVelocity(grunt->physicsBodyId, vel);
}

ze_internal void TickIdle(Ent2d* ent, EntGrunt* grunt, f32 delta)
{
	Ent2d* playerEnt = Sim_FindPlayer();
	BodyState body = ZP_GetBodyState(grunt->physicsBodyId);
	// tick attacking
	if (playerEnt == NULL)
	{
		grunt->aimDegrees = 0;
		return;
	}
	
	// try and walk
	WalkTick(grunt, delta);
	// look at player
	Vec2 pos = body.t.pos;
	BodyState playerBody = ZP_GetBodyState(playerEnt->d.player.physicsBodyId);
	Vec2 aimPos = playerBody.t.pos;
	i32 bHasLos = Sim_CheckLos(pos, aimPos);
	// i32 bHasLos = NO;
	if (bHasLos)
	{
		grunt->aimDegrees = Vec2_AngleTo(pos, aimPos) * RAD2DEG;
	}
	else
	{
		grunt->aimDegrees = 0;
	}
	
	if (grunt->tick <= 0)
	{
		if (!bHasLos)
		{
			grunt->tick = 0.5f;
		}
		else
		{
			grunt->tick = GRUNT_THINK_TIME;
			Sim_SpawnProjectile(
				pos, grunt->aimDegrees, TEAM_ID_ENEMY, PRJ_TEMPLATE_ENEMY_DEFAULT);
		}
	}
	else
	{
		grunt->tick -= delta;
	}
}

ze_internal void Tick(Ent2d* ent, f32 delta)
{
	EntGrunt* grunt = &ent->d.grunt;
	switch (grunt->state)
	{
		case AI_STATE_STUNNED:
		grunt->tick -= delta;
		if (grunt->tick <= 0.f)
		{
			grunt->tick = 0.2f;
			grunt->state = 0;
		}
		break;
		default:
			TickIdle(ent, grunt, delta);
			break;
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
			Transform2d bodyT = ZP_GetBodyPosition(grunt->physicsBodyId);
			Vec2 vel = Vec2_Mul(Vec2_Mul(hit->normal, -1.f), 6.f);
			// kick off the ground regardless of direction of hit
			vel.y += 5.f;
			Sim_SpawnDebris(bodyT.pos, vel, 0.f);
			Ent_MessageOnDeathById(grunt->sourceId, victim->id);
			Remove(victim);
		}
		else
		{
			response.damageDone = hit->damage;
			response.responseType = ENT_HIT_RESPONSE_DAMAGED;
			grunt->state = AI_STATE_STUNNED;
			grunt->tick = 0.4f;
			Vec2 vel = { 0.f, 3.f };
			ZP_SetLinearVelocity(grunt->physicsBodyId, vel);
		}
	}
	return response;
}

ze_internal void GruntPrint(Ent2d* ent)
{
	RNGPRINT("BodyId %d ", ent->d.grunt.physicsBodyId);
}

ze_external void Sim_SpawnEnemyGrunt(Vec2 pos, i32 sourceId)
{
	// pos.x = 0;
	// pos.y = 5;
	//RNGPRINT("Spawn player at %.3f, %.3f\n", pos.x, pos.y);
	EntGruntSave grunt = {};
	grunt.header = Ent_SaveHeaderFromRaw(
		Sim_ReserveDynamicIds(1),
		ENT_EMPTY_TAG,
		ENT_TYPE_ENEMY_GRUNT,
		sizeof(EntGruntSave)
	);
	grunt.pos = pos;
	grunt.health = 70;
	grunt.state = 0;
	grunt.sourceId = sourceId;
	grunt.tick = GRUNT_THINK_TIME;
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

	// configure shared static stuff
	// g_scanLeftOffsetMin = { -0.6f, -0.1f };
}

#include "ze_physics2d_internal.h"

ze_internal b2Vec2 g_gravity(0.0f, -20.0f);
ze_internal b2World g_world(g_gravity);
ze_internal i32 g_velocityIterations = 6;
ze_internal i32 g_positionIterations = 2;

ze_internal ZEBlobStore g_dynamicBodies;
ze_internal ZEBlobStore g_staticBodies;
// id for dynamic bodies. ascends.
ze_internal i32 g_nextDynamicId = 1;
ze_internal i32 g_nextStaticId = -1;

ze_internal ZPVolume2d* GetVolume(zeHandle id)
{
	if (id == 0)
	{
		return NULL;
	}
	if (id > 0)
	{
		return (ZPVolume2d*)g_dynamicBodies.GetById(id);
	}
	else
	{
		return (ZPVolume2d*)g_staticBodies.GetById(id);
	}
}

ze_external Transform2d ZP_GetBodyPosition(zeHandle bodyId)
{
	ZPVolume2d* vol = GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 p = vol->body->GetPosition();
	return { { p.x, p.y }, vol->body->GetAngle() };
}

ze_external void ZP_SetBodyPosition(zeHandle bodyId, Vec2 pos)
{
	ZPVolume2d* vol = GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 p = b2Vec2_FromVec2(pos);
	f32 radians = vol->body->GetAngle();
	vol->body->SetTransform(p, radians);
}

ze_external void ZP_SetLinearVelocity(zeHandle bodyId, Vec2 v)
{
	ZPVolume2d* vol = GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")
	vol->body->SetLinearVelocity(b2Vec2_FromVec2(v));
}

/*ze_external void ZP_SetBodyState(zeHandle bodyId, BodyState state)
{
	ZPVolume2d* vol = GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	vol->body->SetTransform(b2Vec2_FromVec2(state.t.pos), state.t.radians);
	vol->body->SetLinearVelocity(b2Vec2_FromVec2(state.velocity));
}*/

ze_external BodyState ZP_GetBodyState(zeHandle bodyId)
{
	ZPVolume2d* vol = GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 p = vol->body->GetPosition();
	BodyState state = {};
	state.t.pos = Vec2_FromB2Vec2(vol->body->GetPosition());
	state.t.radians = vol->body->GetAngle();
	b2Vec2 v = vol->body->GetLinearVelocity();
	state.velocity = Vec2_FromB2Vec2(v);
	return state;
}

ze_external void ZP_ApplyForce(zeHandle bodyId, Vec2 force)
{
	ZPVolume2d* vol = GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 b2Force;
	b2Force.x = force.x;
	b2Force.y = force.y;
	b2Vec2 point = vol->body->GetPosition();
	vol->body->ApplyForce(b2Force, point, true);
}

internal ZPVolume2d* GetFreeStaticVolume()
{
	ZPVolume2d *volume = (ZPVolume2d*)g_staticBodies.GetFreeSlot(g_nextStaticId);
	volume->id = g_nextStaticId;
	g_nextStaticId -= 1;
	return volume;
}

internal ZPVolume2d* GetFreeDynamicVolume()
{
	ZPVolume2d *volume = (ZPVolume2d*)g_dynamicBodies.GetFreeSlot(g_nextDynamicId);
	volume->id = g_nextDynamicId;
	g_nextDynamicId += 1;
	return volume;
}

ze_external zeHandle ZP_AddStaticVolume(Vec2 pos, Vec2 size)
{
	size = Vec2_Divide(size, 2);
	ZPVolume2d* vol = GetFreeStaticVolume();
	vol->size = size;

	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(pos.x, pos.y);
	vol->body = g_world.CreateBody(&groundBodyDef);
	b2PolygonShape groundBox;
	groundBox.SetAsBox(size.x, size.y);
	b2Fixture* fixture = vol->body->CreateFixture(&groundBox, 0.0f);
	return vol->id;
}

ze_external zeHandle ZP_AddDynamicVolume(ZPShapeDef def)
{
	Vec2 size = Vec2_Divide(def.size, 2);
	ZPVolume2d* vol = GetFreeDynamicVolume();
	vol->size = size;

	b2BodyDef bodyDef;
	bodyDef.fixedRotation = true;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(def.pos.x, def.pos.y);
	vol->body = g_world.CreateBody(&bodyDef);
	b2PolygonShape box;
	box.SetAsBox(size.x, size.y);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.restitution = def.resitition;
	fixtureDef.friction = def.friction;

	b2Fixture* fixture = vol->body->CreateFixture(&fixtureDef);
	return vol->id;
}

ze_external zeHandle ZP_AddBody(ZPBodyDef def)
{
	ZPVolume2d* vol;
	if (def.bIsStatic)
	{
		def.bLockRotation = YES;
		vol = GetFreeStaticVolume();
	}
	else
	{
		vol = GetFreeDynamicVolume();
	}
	ZE_ASSERT(vol != NULL, "ZP_AddBody got no free volume")
	vol->size = def.shape.size;

	b2BodyDef bodyDef;
	bodyDef.fixedRotation = def.bLockRotation;
	bodyDef.type = def.bIsStatic ? b2_staticBody : b2_dynamicBody;
	bodyDef.position.Set(def.shape.pos.x, def.shape.pos.y);

	b2PolygonShape box;
	box.SetAsBox(def.shape.size.x, def.shape.size.y);

	return vol->id;
}

ze_external void ZP_Raycast(Vec2 from, Vec2 to)
{
	RaycastCallback cb;
	b2Vec2 b2From = b2Vec2_FromVec2(from);
	b2Vec2 b2To = b2Vec2_FromVec2(to);
	g_world.RayCast(&cb, b2From, b2To);
}

ze_external void ZPhysicsInit(ZEngine engine)
{
	printf("ZPhysics2d - init\n");
	g_engine = engine;

	ZE_InitBlobStore(g_engine.system.Malloc, &g_dynamicBodies, 1024, sizeof(ZPVolume2d), 0);
	ZE_InitBlobStore(g_engine.system.Malloc, &g_staticBodies, 1024, sizeof(ZPVolume2d), 0);
}

ze_external void ZPhysicsTick(f32 delta)
{
	g_world.Step(delta, g_velocityIterations, g_positionIterations);
	// cleanup blob stores
	g_dynamicBodies.Truncate();
	g_staticBodies.Truncate();
}

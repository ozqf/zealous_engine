#include "ze_physics2d_internal.h"

internal b2Vec2 g_gravity(0.0f, -10.0f);
internal b2World g_world(g_gravity);
internal i32 g_velocityIterations = 6;
internal i32 g_positionIterations = 2;

internal ZEBlobStore g_dynamicBodies;
internal ZEBlobStore g_staticBodies;
// id for dynamic bodies. ascends.
internal i32 g_nextDynamicId = 1;
internal i32 g_nextStaticId = -1;

internal ZPVolume2d* GetVolume(zeHandle id)
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

ze_external Vec2 ZP_GetBodyPosition(zeHandle bodyId)
{
	ZPVolume2d* vol = GetVolume(bodyId);
	if (vol == NULL)
	{
		printf("No volume %d found\n", bodyId);
		ILLEGAL_CODE_PATH
	}
	b2Vec2 p = vol->body->GetPosition();
	return { p.x, p.y };
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

ze_external zeHandle ZP_AddDynamicVolume(Vec2 pos, Vec2 size)
{
	ZPVolume2d* vol = GetFreeDynamicVolume();
	vol->size = size;

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(pos.x, pos.y);
	vol->body = g_world.CreateBody(&bodyDef);
	b2PolygonShape box;
	box.SetAsBox(size.x, size.y);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.restitution = 0.9f;
	fixtureDef.friction = 0.3f;

	b2Fixture* fixture = vol->body->CreateFixture(&fixtureDef);
	return vol->id;
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

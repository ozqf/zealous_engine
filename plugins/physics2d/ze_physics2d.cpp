/*
2D physics box2d implementation
*/
#include "ze_physics2d_internal.h"

#define MAX_DYNAMIC_BODIES 2048
#define MAX_STATIC_BODIES 2048

struct ZPVolume2d
{
	// internally managed reference to this volume
	i32 id;
	// externally provided Id - purely for external user
	i32 externalId;
	// zeHandle handle;
	Vec2 radius;
	b2Body* body;
};

ze_internal ZPVolume2d* ZP_GetVolume(zeHandle id);

ze_internal Vec2 Vec2_FromB2Vec2(b2Vec2 b2v)
{
	return { b2v.x, b2v.y };
}

ze_internal b2Vec2 b2Vec2_FromVec2(Vec2 v)
{
	return b2Vec2(v.x, v.y);
}

ze_internal ZEngine g_engine;

ze_internal b2Vec2 g_gravity(0.0f, -20.0f);
ze_internal b2World g_world(g_gravity);
ze_internal i32 g_velocityIterations = 6;
ze_internal i32 g_positionIterations = 2;

ze_internal ZEBlobStore g_dynamicBodies;
ze_internal ZEBlobStore g_staticBodies;

ze_internal i32 g_nextDynamicId = 1;
ze_internal i32 g_nextStaticId = -1;

// b2QueryCallback implementation
/*class AAABCallback: public b2QueryCallback
{
	public:
	u16 mask;
	bool ReportFixture(b2Fixture* fixture)
	{
		b2Body* body = fixture->GetBody();

	}
};*/

// b2RayCastCallback implementation
// Returns
// -1 to filter, 0 to terminate, fraction to clip the ray for closest hit, 1 to continue 
class RaycastCallback: public b2RayCastCallback
{
	public:
	// b2Fixture* closest;
	// this stuff is externally allocated!
	ZPRaycastResult* results;
	i32 maxResults;
	i32 numResults;
	u16 mask;
	// f32 fraction;

	void SetResultsArray(ZPRaycastResult* newResults, i32 max)
	{
		results = newResults;
		maxResults = max;
		numResults = 0;
	}

	f32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, f32 fraction)
	{
		// printf("Ray hit, fraction %.3f\n", fraction);
		if (numResults == maxResults) { return fraction; }

		// filter
		if ((fixture->GetFilterData().maskBits & this->mask) == 0)
		{
			return fraction;
		}

		ZPRaycastResult* r = &results[numResults];
		*r = {};
		numResults += 1;
		r->pos = Vec2_FromB2Vec2(point);
		r->normal = Vec2_FromB2Vec2(normal);
		r->fraction = fraction;
		r->categoryBits = fixture->GetFilterData().categoryBits;
		zeHandle volId = (zeHandle)fixture->GetBody()->GetUserData().pointer;
		ZPVolume2d* vol = (ZPVolume2d*)ZP_GetVolume(volId);
		if (vol != NULL)
		{
			r->volumeId = vol->id;
			r->externalId = vol->externalId;
		}
		else
		{
			r->volumeId = 0;
			r->externalId = 0;
		}
		return fraction;
	}
};

ze_internal void ZP_CrashDump()
{
	
	printf("--- ZPhysics crash dump ---\n");
	i32 numDynamic = g_dynamicBodies.Count();
	i32 numStatic = g_staticBodies.Count();
	printf("Dynamic bodies:\n");
	for (i32 i = 0; i < numDynamic; ++i)
	{
		ZPVolume2d* vol = (ZPVolume2d*)g_dynamicBodies.GetByIndex(i);
		printf("%d: bodyId %d, external Id %d.\n",
			i, vol->id, vol->externalId);
	}
	// for (i32 i = 0; i < numStatic; ++i)
	// {
	// 	ZPVolume2d* vol = (ZPVolume2d*)g_dynamicBodies.GetByIndex(i);
	// 	printf("%d: bodyId %d, external Id %d.\n",
	// 		i, vol->id, vol->externalId);
	// }
}

ze_internal ZPVolume2d* ZP_GetVolume(zeHandle id)
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

ze_external zErrorCode ZP_RemoveBody(zeHandle bodyId)
{
	if (bodyId == 0) { return ZE_ERROR_NOT_FOUND; }

	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	g_world.DestroyBody(vol->body);
	if (bodyId > 0)
	{
		g_dynamicBodies.MarkForRemoval(bodyId);
		// TODO: This Truncate could break stuff if Remove is called during iteration
		// of bodies. Consider flagging bodies and removing them before the next step
		g_dynamicBodies.Truncate();
	}
	else
	{
		g_staticBodies.MarkForRemoval(bodyId);
		// TODO: This Truncate could break stuff if Remove is called during iteration
		// of bodies. Consider flagging bodies and removing them before the next world step
		g_dynamicBodies.Truncate();
	}
	return ZE_ERROR_NONE;
}

ze_external Transform2d ZP_GetBodyPosition(zeHandle bodyId)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 p = vol->body->GetPosition();
	return { { p.x, p.y }, vol->body->GetAngle() };
}

ze_external void ZP_SetBodyPosition(zeHandle bodyId, Vec2 pos)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 p = b2Vec2_FromVec2(pos);
	f32 radians = vol->body->GetAngle();
	vol->body->SetTransform(p, radians);
}

ze_external void ZP_SetLinearVelocity(zeHandle bodyId, Vec2 v)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")
	vol->body->SetLinearVelocity(b2Vec2_FromVec2(v));
}

ze_external void ZP_SetBodyState(zeHandle bodyId, BodyState state)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	vol->body->SetTransform(b2Vec2_FromVec2(state.t.pos), state.t.radians);
	vol->body->SetLinearVelocity(b2Vec2_FromVec2(state.velocity));
	vol->body->SetAngularVelocity(state.angularVelocity);
}

ze_external BodyState ZP_GetBodyState(zeHandle bodyId)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	//ZE_ASSERT(vol != NULL, "Body is null")
	if (vol == NULL)
	{
		ZE_BUILD_STRING(msg, 256, "Body %d not found", bodyId);
		ZE_ASSERT(NO, msg)
	}

	b2Vec2 p = vol->body->GetPosition();
	BodyState state = {};
	state.t.pos = Vec2_FromB2Vec2(vol->body->GetPosition());
	state.t.radians = vol->body->GetAngle();
	b2Vec2 v = vol->body->GetLinearVelocity();
	state.velocity = Vec2_FromB2Vec2(v);
	state.angularVelocity = vol->body->GetAngularVelocity();
	state.externalId = vol->externalId;
	return state;
}

ze_external void ZP_ApplyForceAtPoint(zeHandle bodyId, Vec2 force, Vec2 point)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 b2Force;
	b2Force.x = force.x;
	b2Force.y = force.y;
	b2Vec2 b2Point = b2Vec2_FromVec2(point);
	vol->body->ApplyForce(b2Force, b2Point, true);
}

ze_external void ZP_ApplyForce(zeHandle bodyId, Vec2 force)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")

	b2Vec2 b2Force = b2Vec2_FromVec2(force);
	b2Vec2 point = vol->body->GetPosition();
	vol->body->ApplyForceToCenter(b2Force, true);
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

ze_external zeHandle ZP_AddStaticVolume(Vec2 pos, Vec2 size, u16 categoryBits, u16 maskBits)
{
	size = Vec2_Divide(size, 2);
	ZPVolume2d* vol = GetFreeStaticVolume();
	vol->radius = size;
	vol->externalId = 0;

	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(pos.x, pos.y);
	vol->body = g_world.CreateBody(&groundBodyDef);
	b2PolygonShape groundBox;
	groundBox.SetAsBox(size.x, size.y);
	b2Fixture* fixture = vol->body->CreateFixture(&groundBox, 0.0f);
	b2Filter filter;
	filter.categoryBits = categoryBits;
	filter.maskBits = maskBits;
	fixture->SetFilterData(filter);
	return vol->id;
}

ze_external zeHandle ZP_AddBody(ZPBodyDef def)
{
	ZPVolume2d* vol = NULL;
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
	vol->radius = def.shape.radius;
	vol->externalId = def.externalId;

	if (def.categoryBits == 0)
	{
		def.categoryBits = 1;
	}
	if (def.maskBits == 0)
	{
		def.maskBits = 65535;
	}

	b2BodyDef bodyDef;
	bodyDef.fixedRotation = def.bLockRotation;
	bodyDef.type = def.bIsStatic ? b2_staticBody : b2_dynamicBody;
	bodyDef.position.Set(def.shape.pos.x, def.shape.pos.y);
	bodyDef.userData.pointer = (uintptr_t)vol->id;
	vol->body = g_world.CreateBody(&bodyDef);
	
	b2PolygonShape box;
	box.SetAsBox(def.shape.radius.x, def.shape.radius.y);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.restitution = def.resitition;
	fixtureDef.friction = def.friction;
	#if 0 // verbose
	printf("Create body %d: mask %d category %d\n",
		vol->id, fixtureDef.filter.maskBits, fixtureDef.filter.categoryBits);
	#endif

	b2Fixture* fixture = vol->body->CreateFixture(&fixtureDef);
	return vol->id;
}

ze_external i32 ZP_Raycast(
	Vec2 from, Vec2 to, ZPRaycastResult* results, i32 maxResults, u16 mask)
{
	if (mask == 0)
	{
		mask = ZP_MASK_ALL;
	}
	RaycastCallback cb;
	b2Vec2 b2From = b2Vec2_FromVec2(from);
	b2Vec2 b2To = b2Vec2_FromVec2(to);
	cb.SetResultsArray(results, maxResults);
	cb.mask = mask;
	g_world.RayCast(&cb, b2From, b2To);
	return cb.numResults;
}

ze_external i32 ZP_AabbCast(
	Vec2 min, Vec2 max, i32 maxResults, u16 mask)
{

	return 0;
}

ze_external void ZPhysicsInit(ZEngine engine)
{
	printf("ZPhysics2d - init\n");
	g_engine = engine;
	ZE_SetFatalError(g_engine.system.Fatal);
	g_engine.system.RegisterCrashDumpFunction(ZP_CrashDump);
	
	size_t blobSize = sizeof(ZPVolume2d);
	ZE_mallocFunction mallocFn = g_engine.system.Malloc;
	ZE_InitBlobStore(mallocFn, &g_dynamicBodies, MAX_DYNAMIC_BODIES, blobSize, 0);
	ZE_InitBlobStore(mallocFn, &g_staticBodies, MAX_STATIC_BODIES, blobSize, 0);
}

ze_external void ZPhysicsTick(f32 delta)
{
	g_world.Step(delta, g_velocityIterations, g_positionIterations);
	// cleanup blob stores
	g_dynamicBodies.Truncate();
	g_staticBodies.Truncate();
}

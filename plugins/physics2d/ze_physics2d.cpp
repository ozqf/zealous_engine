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
	ZPBodyDef def;
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
class AABBCallback: public b2QueryCallback
{
	public:
	u16 mask;
	// this stuff is externally allocated!
	ZAABBResult* results;
	i32 numResults;
	i32 maxResults;

	bool ReportFixture(b2Fixture* fixture)
	{
		b2Body* body = fixture->GetBody();
		u16 bodyCategory = fixture->GetFilterData().categoryBits;
		if ((bodyCategory & this->mask) == 0)
		{
			return true;
		}

		ZAABBResult* r = &results[numResults];
		numResults += 1;
		*r = {};
		zeHandle volId = (zeHandle)body->GetUserData().pointer;
		ZPVolume2d* vol = (ZPVolume2d*)ZP_GetVolume(volId);
		if (vol != NULL)
		{
			r->volumeId = vol->id;
			r->externalId = vol->externalId;
		}
		else
		{
			r->volumeId = ZP_EMPTY_ID;
			r->externalId = ZP_EMPTY_ID;
		}
		if (numResults >= maxResults)
		{
			return false;
		}
		return true;
	}
};

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

	// Called for each fixture found in the query. You control how the ray cast proceeds by the return value:
	// return -1: ignore this fixture and continue
	// return 0: terminate the ray cast
	// return fraction: clip the ray to this point
	// return 1: don't clip the ray and continue
	f32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, f32 fraction)
	{
		// printf("Ray hit, fraction %.3f\n", fraction);
		if (numResults == maxResults) { return 0; }

		u16 categoryBits = fixture->GetFilterData().categoryBits;
		// filter
		if ((categoryBits & this->mask) == 0)
		{
			// printf("Filter mismatch: %d vs query %d\n",
			// 	categoryBits, mask);
			// return fraction;
			return -1;
		}

		ZPRaycastResult* r = &results[numResults];
		*r = {};
		numResults += 1;
		r->pos = Vec2_FromB2Vec2(point);
		r->normal = Vec2_FromB2Vec2(normal);
		r->fraction = fraction;
		r->categoryBits = categoryBits;
		zeHandle volId = (zeHandle)fixture->GetBody()->GetUserData().pointer;
		ZPVolume2d* vol = (ZPVolume2d*)ZP_GetVolume(volId);
		if (vol != NULL)
		{
			r->volumeId = vol->id;
			r->externalId = vol->externalId;
		}
		else
		{
			r->volumeId = ZP_EMPTY_ID;
			r->externalId = ZP_EMPTY_ID;
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
		printf("Destroying static body %d\n", bodyId);
		g_staticBodies.MarkForRemoval(bodyId);
		// TODO: This Truncate could break stuff if Remove is called during iteration
		// of bodies. Consider flagging bodies and removing them before the next world step
		g_staticBodies.Truncate();
	}
	return ZE_ERROR_NONE;
}

ze_external void ZP_SetBodyMaskBit(zeHandle bodyId, u16 maskBit, i32 bOn)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	if (vol == NULL) { return; }
	if (bOn)
	{
		vol->def.maskBits |= maskBit;
	}
	else
	{
		vol->def.maskBits &= ~maskBit;
	}
	b2Filter filter = vol->body->GetFixtureList()->GetFilterData();
	filter.maskBits = vol->def.maskBits;
	vol->body->GetFixtureList()->SetFilterData(filter);
}

ze_external ZPShapeDef ZP_GetBodyShape(zeHandle bodyId)
{
	ZPVolume2d* vol = ZP_GetVolume(bodyId);
	ZE_ASSERT(vol != NULL, "Body is null")
	return vol->def.shape;
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
	vol->externalId = 0;
	vol->def.shape.radius = size;
	vol->def.shape.shapeType = 0;
	// todo: setup volume's shape def?

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
	vol->externalId = def.externalId;
	vol->def = def;

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
	fixtureDef.filter.categoryBits = def.categoryBits;
	fixtureDef.filter.maskBits = def.maskBits;
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

ze_external i32 ZP_AABBCast(
	Vec2 min, Vec2 max, ZAABBResult* results, i32 maxResults, u16 mask)
{
	b2AABB aabb;
	aabb.lowerBound.Set(min.x, min.y);
	aabb.upperBound.Set(max.x, max.y);
	AABBCallback cb;
	cb.results = results;
	cb.numResults = 0;
	cb.maxResults = maxResults;
	cb.mask = mask;
	g_world.QueryAABB(&cb, aabb);
	return cb.numResults;
}

ze_external i32 ZP_GroundTest(zeHandle physicsBody, u16 mask)
{
	const f32 tolerance = 0.1f;
    Transform2d t = ZP_GetBodyPosition(physicsBody);
	ZPShapeDef shape = ZP_GetBodyShape(physicsBody);
	Vec2 groundQuery = Vec2_Add(t.pos, { 0, -shape.radius.y });
	Vec2 min = groundQuery, max = groundQuery;
	min = groundQuery;
	max = groundQuery;
	min.x -= (shape.radius.x - tolerance);
	min.y -= tolerance;
	max.x += (shape.radius.x - tolerance);
	max.y += tolerance;
	// printf("Ground check. Mask %d, body at %.3f, %.3f. Area: %.3f, %.3f to %.3f, %.3f\n",
	// 	mask, t.pos.x, t.pos.y, min.x, min.y, max.x, max.y);

	const i32 numResults = 16;
	ZAABBResult results[numResults];
	i32 count = ZP_AABBCast(min, max, results, numResults, mask);
	return (count > 0);
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

ze_external ZPStats ZPhysicsStats()
{
	ZPStats stats = {};
	stats.numRegisteredDynamic = g_dynamicBodies.Count();
	stats.numRegisteredStatic = g_staticBodies.Count();
	stats.numBodies = g_world.GetBodyCount();
	return stats;
}

ze_external void ZPhysicsClearAll()
{

}

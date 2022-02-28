#ifndef ZE_PHYSICS2D_INTERNAL_H
#define ZE_PHYSICS2D_INTERNAL_H
/*
2D physics box2d implementation - private header
*/
#include "../ze_physics2d.h"

// Box2d must be built in 64 bit, using embedded standard library and exception disabled.
#include "../../lib/box2d/box2d.h"

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

ze_internal Vec2 Vec2_FromB2Vec2(b2Vec2 b2v)
{
	return { b2v.x, b2v.y };
}

ze_internal b2Vec2 b2Vec2_FromVec2(Vec2 v)
{
	return b2Vec2(v.x, v.y);
}

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
		ZPRaycastResult* r = &results[numResults];
		*r = {};
		numResults += 1;
		r->pos = Vec2_FromB2Vec2(point);
		r->normal = Vec2_FromB2Vec2(normal);
		r->fraction = fraction;
		ZPVolume2d* vol = (ZPVolume2d*)fixture->GetBody()->GetUserData().pointer;
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

ze_internal ZEngine g_engine;

#endif ZE_PHYSICS2D_INTERNAL_H

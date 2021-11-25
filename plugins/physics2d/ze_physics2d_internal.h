#ifndef ZE_PHYSICS2D_INTERNAL_H
#define ZE_PHYSICS2D_INTERNAL_H

#include "../ze_physics2d.h"

// Box2d must be built in 64 bit, using embedded standard library and exception disabled.
#include "../../lib/box2d/box2d.h"

struct ZPVolume2d
{
	i32 id;
	// zeHandle handle;
	Vec2 radius;
	b2Body* body;
};

// b2RayCastCallback implementation
// Returns
// -1 to filter, 0 to terminate, fraction to clip the ray for closest hit, 1 to continue 
class RaycastCallback: public b2RayCastCallback
{
	public:
	b2Fixture* m_closest;
	f32 m_fraction;

	f32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, f32 fraction)
	{
		printf("Ray hit, fraction %.3f\n", fraction);
		return fraction;
	}
};

ze_internal Vec2 Vec2_FromB2Vec2(b2Vec2 b2v)
{
	return { b2v.x, b2v.y };
}

ze_internal b2Vec2 b2Vec2_FromVec2(Vec2 v)
{
	return b2Vec2(v.x, v.y);
}

ze_internal ZEngine g_engine;

#endif ZE_PHYSICS2D_INTERNAL_H

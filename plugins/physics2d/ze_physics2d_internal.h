#ifndef ZE_PHYSICS2D_INTERNAL_H
#define ZE_PHYSICS2D_INTERNAL_H

#include "../ze_physics2d.h"

// Box2d must be built in 64 bit, using embedded standard library and exception disabled.
#include "../../lib/box2d/box2d.h"

struct ZPVolume2d
{
	i32 id;
	zeHandle handle;
	Vec2 size;
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

ze_internal ZEngine g_engine;

#endif ZE_PHYSICS2D_INTERNAL_H

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

internal ZEngine g_engine;

#endif ZE_PHYSICS2D_INTERNAL_H

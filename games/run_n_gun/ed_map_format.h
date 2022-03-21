#include "../../headers/zengine.h"

struct Map2dAABB
{
	i32 id;
	Vec2 min;
	Vec2 max;
};

struct Map2d
{
	i32 volumesOffset;
	i32 numVolumes;
};

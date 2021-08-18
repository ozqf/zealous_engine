#ifndef ZE_BVH_H
#define ZE_BVH_H

#include "../ze_common.h"

#define BVH_INVALID_ID 0

struct ZEBVHNode
{
	i32 id;
	AABB aabb;
	i32 leftId;
	i32 rightId;
};

struct ZEBVH
{
	i32 nextId;
	ZEBVHNode* root;
};

internal i32 ZEBVH_Insert(ZEBVH* bvh, AABB aabb)
{
	return BVH_INVALID_ID;
}

internal void ZEBVH_Create(
	ZE_mallocFunction allocFn,
	ZE_freeFunction freeFn)
{
	
}

#endif // ZE_BVH_H
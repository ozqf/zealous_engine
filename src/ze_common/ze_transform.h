#ifndef ZE_TRANSFORM_H
#define ZE_TRANSFORM_H

#include "ze_common.h"

/////////////////////////////////////////////////////////////////////
// Position
/////////////////////////////////////////////////////////////////////
static void Transform_SetToIdentity(Transform* t)
{
	*t = {};
    t->pos = { 0, 0, 0 };
    t->scale = { 1, 1, 1 };
    M3x3_SetToIdentity(t->rotation.cells);
}

#endif // ZE_TRANSFORM_H
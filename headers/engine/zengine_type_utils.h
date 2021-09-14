#ifndef ZENGINE_TYPE_UTILS_H
#define ZENGINE_TYPE_UTILS_H

#include "../zengine.h"

ze_internal void ZRDrawObj_AddLineVert(ZRDrawObjData* lineData, Vec3 pos)
{
	if (lineData == NULL) { return; }
	if (lineData->lines.numVerts >= lineData->lines.maxVerts) { return; }
	lineData->lines.verts[lineData->lines.numVerts++].pos = pos;
}

ze_internal Vec3 M3x3_Calculate3DMove(M3x3* rotation, Vec3 input)
{
	Vec3 forward = rotation->zAxis;
    Vec3 left = rotation->xAxis;
    Vec3 up = rotation->yAxis;
    Vec3 result = {};
    result.x += forward.x * input.z;
    result.y += forward.y * input.z;
    result.z += forward.z * input.z;

    result.x += left.x * input.x;
    result.y += left.y * input.x;
    result.z += left.z * input.x;

    result.x += up.x * input.y;
    result.y += up.y * input.y;
    result.z += up.z * input.y;

    Vec3_Normalise(&result);
    return result;
}

#endif // ZENGINE_TYPE_UTILS_H

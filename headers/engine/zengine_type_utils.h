#ifndef ZENGINE_TYPE_UTILS_H
#define ZENGINE_TYPE_UTILS_H

#include "../zengine.h"

ze_internal void ZRDrawObj_AddLineVert(ZRDrawObjData* lineData, Vec3 pos)
{
	if (lineData == NULL) { return; }
	if (lineData->lines.numVerts >= lineData->lines.maxVerts) { return; }
	lineData->lines.verts[lineData->lines.numVerts++].pos = pos;
}

#endif // ZENGINE_TYPE_UTILS_H

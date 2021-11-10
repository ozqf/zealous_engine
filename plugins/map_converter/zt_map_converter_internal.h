#ifndef ZT_MAP_CONVERTER_INTERNAL_H
#define ZT_MAP_CONVERTER_INTERNAL_H

// public
#include "../zt_map_converter.h"

// embedded examples
#include "map_format_example_128x128x32_cube.h"

// internal
#include <stdlib.h>

/*struct ZPlane
{
	Vec3 normal;
	Vec3 point;
};*/

struct ZPlane
{
	Vec3 normal;
	f32 d; // distance from origin
};

struct ZTMapFace
{
	// note that raw coords from a .map z is vertical, not y
	Vec3 a, b, c;
	Vec2 texOrigin;
	f32 texRotation;
	char* texture;
};

struct ZTMapBrush
{
	i32 firstFaceIndex;
	i32 numFaces;
};

struct ZTMapFile
{
	ZTMapFace* faces;
	i32 numFaces;
	i32 maxFaces;
	ZTMapBrush* brushes;
	i32 numBrushes;
	i32 maxBrushes;
};

static void DebugPrintFileData(ZTMapBrush* brushes, i32 numBrushes, ZTMapFace* faces, i32 numFaces);
static void DebugPrintFace(ZTMapFace* face);

static void IfGreaterUpdateLimit(f32 val, f32* limit)
{
	if (val <= *limit) { return; }
	*limit = val;
}

static void IfLesserUpdateLimit(f32 val, f32* limit)
{
	if (val >= *limit) { return; }
	*limit = val;
}

static void CalcBrushExtends(ZTMapBrush* b, ZTMapFace* faces)
{
	Vec3 min = { 999999, 999999, 999999 };
	Vec3 max = { -999999, -999999, -999999 };
	i32 first = b->firstFaceIndex;
	for (i32 i = 0; i < b->numFaces; ++i)
	{
		i32 index = first + i;
		ZTMapFace* f = &faces[index];
		IfGreaterUpdateLimit(f->a.x, &max.x);
		IfGreaterUpdateLimit(f->a.y, &max.y);
		IfGreaterUpdateLimit(f->a.z, &max.z);
		
		IfLesserUpdateLimit(f->a.x, &min.x);
		IfLesserUpdateLimit(f->a.y, &min.y);
		IfLesserUpdateLimit(f->a.z, &min.z);
	}
	printf("Brush Extents: %.1f, %.1f, %.1f to %.1f, %.1f, %.1f\n",
		min.x, min.y, min.z, max.x, max.y, max.z);
}

#endif // ZT_MAP_CONVERTER_INTERNAL_H
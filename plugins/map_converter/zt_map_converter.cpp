#ifndef ZT_MAP_CONVERTER_CPP
#define ZT_MAP_CONVERTER_CPP

// https://github.com/stefanha/map-files
// https://github.com/stefanha/map-files/blob/master/map.cpp
// https://github.com/stefanha/map-files/blob/f28e6a33b41e8748990e092f2878add5c1922cb8/math.h
// http://www.songho.ca/math/plane/plane.html

#include "zt_map_converter_internal.h"
#include "zt_map_loader.h"

ze_internal void DebugPrintPlane(ZPlane* p)
{
	printf("Normal: %.3f, %.3f, %.3f - D: %.3f\n",
		p->normal.x, p->normal.y, p->normal.z, p->d);
}

static ZPlane PlaneFromBrushFace(const ZTMapFace* face)
{
	ZPlane p = {};
	p.normal = Vec3_CrossProduct(Vec3_Subtract(face->c, face->b), Vec3_Subtract(face->a, face->b));
	Vec3_Normalise(&p.normal);
	p.d = Vec3_DotProduct(Vec3_Flipped(p.normal), face->a);
	return p;
}

static i32 IntersectFaces(ZTMapFace* faceA, ZTMapFace* faceB, ZTMapFace* faceC, Vec3* result)
{
	ZPlane a = PlaneFromBrushFace(faceA);
	ZPlane b = PlaneFromBrushFace(faceB);
	ZPlane c = PlaneFromBrushFace(faceC);

	// n1.Dot(n2.Cross(n3));
	f32 denom = Vec3_DotProduct(a.normal, Vec3_CrossProduct(b.normal, c.normal));
	if (denom == 0)
	{
		// no intersection
		return NO;
	}

	// printf("Intersecting planes:\n");
	// DebugPrintPlane(&a);
	// DebugPrintPlane(&b);
	// DebugPrintPlane(&c);
	// calc result
	// p = -d1 * (n2.Cross(n3)) - d2 * (n3.Cross(n1)) - d3 * (n1.Cross(n2)) / denom;
	Vec3 n2n3Cross = Vec3_CrossProduct(b.normal, c.normal);
	Vec3 n3n1Cross = Vec3_CrossProduct(c.normal, a.normal);
	Vec3 n1n2Cross = Vec3_CrossProduct(a.normal, b.normal);

	
	result->x = -a.d * n2n3Cross.x - b.d * n3n1Cross.x - c.d * n1n2Cross.x / denom;
	result->y = -a.d * n2n3Cross.y - b.d * n3n1Cross.y - c.d * n1n2Cross.y / denom;
	result->z = -a.d * n2n3Cross.z - b.d * n3n1Cross.z - c.d * n1n2Cross.z / denom;

	return YES;
}

static ErrorCode BrushToVertices(ZTMapFile* map, i32 brushIndex, ZTMapOutput* output)
{
	ErrorCode err = ZE_ERROR_NONE;
	ZTMapBrush* brush = &map->brushes[brushIndex];
	i32 first = brush->firstFaceIndex;
	i32 len = brush->numFaces;
	printf("Brush %d has %d faces\n", brushIndex, len);
	i32 intersectionsFound = 0;
	i32 intersectionsSkipped = 0;
	
	for (i32 i = 0; i < len; ++i)
	{
		i32 numPoints = 0;
		for (i32 j = 0; j < len; ++j)
		{
			for (i32 k = 0; k < len; ++k)
			{
				if (i != j != k)
				{
					// process
					ZTMapFace* fa = &map->faces[first + i];
					ZTMapFace* fb = &map->faces[first + j];
					ZTMapFace* fc = &map->faces[first + k];
					Vec3 point;
					err = IntersectFaces(fa, fb, fc, &point);
					if (err == NO)
					{
						intersectionsSkipped += 1;
						continue;
					}
					numPoints += 1;
					intersectionsFound += 1;
					printf("intersect faces %d, %d, %d\n",
						first + i,
						first + j,
						first + k);
					// DebugPrintFace(fa);
					// DebugPrintFace(fb);
					// DebugPrintFace(fc);
					printf("\tintersection at %.3f, %.3f, %.3f\n",
						point.x, point.y, point.z);
					output->verts[output->numVerts] = point;
					output->numVerts += 1;
				}
			}
		}
		printf("Brush %d face %d has %d points\n",
			brushIndex, i, numPoints);
	}
	printf("Intersections found: %d vs skipped: %d\n",
		intersectionsFound, intersectionsSkipped);
	return ZE_ERROR_NONE;
}

static ErrorCode ConvertToTriSoup(ZTMapFile* map, ZTMapOutput* output)
{
	// No validity checking here - assuming we have brushes and faces
	printf("Converting %d brushes to trisoup\n", map->numBrushes);
	for (i32 i = 0; i < map->numBrushes; ++i)
	{
		ZTMapBrush* brush = &map->brushes[i];
		CalcBrushExtends(brush, map->faces);
		// calculate face intersections
		BrushToVertices(map, i, output);
	}
	printf("Read %d brush vertices\n", output->numVerts);
	return 0;
}

/////////////////////////////////////////////////////
// Public
/////////////////////////////////////////////////////
ze_external zErrorCode ZT_MapConvert(const char* mapText)
{
	/*
	printf("Convert Map\n");
	ZTMapFile map = {};
	printf("Parse map file\n");
	const char* path = "map_format_example_128x128x32_cube.map";
	zErrorCode err = ParseMapFromFile(path, &map);
	
	if (err != 0)
	{
		printf("Error %d parsing map file\n", err);
		return err;
	}
	
	DebugPrintFileData(
		map.brushes,
		map.numBrushes,
		map.faces,
		map.numFaces);
	
	printf("Done\n");
	
	err = ConvertToTriSoup(&map);
	if (err != 0)
	{
		printf("Error %d converting brushes\n", err);
		return err;
	}
	
	printf("Done\n");
	*/
	return ZE_ERROR_NONE;
}

ze_external zErrorCode ZT_MapConvertTest(ZTMapOutput** userOutput)
{
	ZTMapOutput* output = (ZTMapOutput*)malloc(sizeof(ZTMapOutput));
	output->maxVerts = 10000;
	output->verts = (Vec3*)malloc(sizeof(Vec3) * output->maxVerts);
	output->numVerts = 0;
	

	ZTMapFile map;
	
	// Load map file into heap
	zErrorCode err = ZT_ParseMapFromText(map_format_example_128x128x32_cube_h, &map);
	printf("Map test result code: %d\n", err);
	DebugPrintFileData(map.brushes, map.numBrushes, map.faces, map.numFaces);
	
	// convert from brushes to triangles
	err = ConvertToTriSoup(&map, output);
	if (err != 0)
	{
		printf("Error %d converting brushes\n", err);
		return err;
	}
	
	return err;
}

ze_external zErrorCode ZT_MapConvertFree(ZTMapOutput* output)
{
	return ZE_ERROR_NONE;
}

#endif // ZT_MAP_CONVERTER_CPP
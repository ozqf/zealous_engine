#ifndef ZT_MAP_CONVERTER_CPP
#define ZT_MAP_CONVERTER_CPP

// https://github.com/stefanha/map-files

#include "zt_map_converter_internal.h"
#include "zt_map_loader.h"

struct ZPlane
{
	Vec3 normal;
	Vec3 point;
};

static void IntersectFaces(ZTMapFace* a, ZTMapFace* b)
{
	printf("intersect faces:\n");
	DebugPrintFace(a);
	DebugPrintFace(b);
}

static ErrorCode ConvertToTriSoup(ZTMapFile* map)
{
	// No validity checking here - assuming we have brushes and faces
	printf("Converting %d brushes to trisoup\n", map->numBrushes);
	ZTMapBrush* brush = &map->brushes[0];
	
	CalcBrushExtends(brush, map->faces);
	
	i32 i1 = brush->firstFaceIndex;
	i32 i2 = brush->firstFaceIndex + 1;
	ZTMapFace* a = &map->faces[i1];
	ZTMapFace* b = &map->faces[i2];
	IntersectFaces(a, b);
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

ze_external zErrorCode ZT_MapConvertTest()
{
	ZTMapFile map;
	
	// Load map file into heap
	zErrorCode err = ZT_ParseMapFromText(map_format_example_128x128x32_cube_h, &map);
	printf("Map test result code: %d\n", err);
	DebugPrintFileData(map.brushes, map.numBrushes, map.faces, map.numFaces);

	
	err = ConvertToTriSoup(&map);
	if (err != 0)
	{
		printf("Error %d converting brushes\n", err);
		return err;
	}
	
	return err;
}

#endif // ZT_MAP_CONVERTER_CPP
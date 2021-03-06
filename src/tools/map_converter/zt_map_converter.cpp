#ifndef ZT_MAP_CONVERTER_CPP
#define ZT_MAP_CONVERTER_CPP

#include "zt_map_converter_internal.h"
#include "zt_map_loader.h"

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
extern "C" ErrorCode ZT_MapConvert(const char* filePath)
{
	printf("Convert Map\n");
	ZTMapFile map = {};
	printf("Parse map file\n");
	ErrorCode err = ParseMapFile(&map);
	
	if (err != 0)
	{
		printf("Error %d parsing map file\n", err);
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
	}
	
	printf("Done\n");
	
	return 0;
}

#endif // ZT_MAP_CONVERTER_CPP
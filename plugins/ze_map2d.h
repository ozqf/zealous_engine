#ifndef ZE_MAP2D_H
#define ZE_MAP2D_H

#include "../../headers/zengine.h"

#define MAP2D_EMBEDDED_MAP_0 0
#define MAP2D_EMBEDDED_MAP_1 1
#define MAP2D_EMBEDDED_MAP_2 2
#define MAP2D_EMBEDDED_MAP_GUNN_TEST_A 3

struct Map2dAABB
{
	i32 id;
    i32 type;
	Vec2 min;
	Vec2 max;
};

struct Map2dLine
{
	i32 id;
    i32 type;
	Vec2 a;
	Vec2 b;
};

struct Map2dEntity
{
    i32 id;
    Vec2 pos;
    zeSize typeStrOffset;
};

/*
Header|aabbs|lines|ents|strings
*/
struct Map2d
{
    char magic[4];
    zeSize totalBytes;
	
    zeSize offsetAABBs;
	i32 numAABBs;

    zeSize offsetLines;
    i32 numLines;

    zeSize offsetEnts;
    i32 numEnts;
    
    zeSize offsetChars;
    i32 numChars;
    zeSize offsetCursorChars;
};

// helper struct when reading from a Map2d
struct Map2dReader
{
    Map2dAABB* aabbs;
    i32 numAABBs;
    Map2dLine* lines;
    i32 numLines;
    Map2dEntity* ents;
    i32 numEnts;
    char* chars;
    i32 numChars;
};

ze_external Map2d* Map2d_ReadEmbedded(i32 index);
ze_external Map2dReader Map2d_CreateReader(Map2d* map);
ze_external void Map2d_Free(Map2d* map);
ze_external void Map2d_Init(ZEngine engine);
ze_external void Map2d_DebugDump(Map2d* map);

#endif // ZE_MAP2D_H

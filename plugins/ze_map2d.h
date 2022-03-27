#include "../../headers/zengine.h"

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

/*struct Map2dPlatform
{
    i32 minX;
    i32 maxX;
    i32 y;
};*/

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

ze_external Map2d* Map2d_ReadEmbedded();
ze_external Map2dReader Map2d_CreateReader(Map2d* map);
ze_external void Map2d_Free(Map2d* map);
ze_external void Map2d_Init(ZEngine engine);
ze_external void Map2d_DebugDump(Map2d* map);

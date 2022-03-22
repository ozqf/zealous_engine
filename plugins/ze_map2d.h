#include "../../headers/zengine.h"

struct Map2dAABB
{
	i32 id;
    i32 type;
	Vec2 min;
	Vec2 max;
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

struct Map2d
{
    char magic[4];
    zeSize totalBytes;
	zeSize offsetAABBs;
	i32 numAABBs;
    zeSize offsetEnts;
    i32 numEnts;
    zeSize offsetChars;
    i32 numChars;
};

ze_external Map2d* Map2d_ReadEmbedded();
ze_external void Map2d_Free(Map2d* map);
ze_external void Map2d_Init(ZEngine engine);
ze_external void Map2d_DebugDump(Map2d* map);

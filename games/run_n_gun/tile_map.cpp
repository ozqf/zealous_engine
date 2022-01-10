#include "tile_map.h"
/*
Tile maps are grids of tile materials.
The material defines the properties of the tiles.
*/


struct TileMaterial
{
	i32 id;
	i8* name;
	i32 layerId;
	i32 paintId;
	i32 collisionType;

	i32 materialId;
};

struct TileCell
{
	i32 bUsed;
	i32 seed;
};

struct TileMap
{
	Vec2 origin;
	i32 width;
	i32 height;
	TileCell hull[64];
};

ze_internal ZEngine g_engine;



ze_external void TilesInit(ZEngine engine)
{
	g_engine = engine;
}

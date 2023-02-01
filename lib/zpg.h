#ifndef ZPG_H
#define ZPG_H

#ifdef ZPG_WINDOWS_DLL 
#define ZPG_EXPORT extern "C" __declspec(dllexport)
#else
#define ZPG_EXPORT extern "C"
#endif

#include "zpg/zpg_common.h"
#include <stdlib.h>

#define ZPG_OUTPUT_TYPE_ASCI_GRID (1 << 0)
#define ZPG_OUTPUT_TYPE_GREYSCALE (1 << 1)

#define ZPG_API_FLAG_PRINT_RESULT (1 << 0)
#define ZPG_API_FLAG_PRINT_WORKING (1 << 1)
#define ZPG_API_FLAG_STEP_THROUGH (1 << 2)
#define ZPG_API_FLAG_NO_ENTITIES (1 << 3)
#define ZPG_API_FLAG_PRINT_GREYSCALE (1 << 4)
#define ZPG_API_FLAG_PRINT_FINAL_ALLOCS (1 << 5)
#define ZPG_API_FLAG_SOLID_BORDER (1 << 6)

#define ZPG_FLAG_ABOVE_LEFT (1 << 0)
#define ZPG_FLAG_ABOVE (1 << 1)
#define ZPG_FLAG_ABOVE_RIGHT (1 << 2)
#define ZPG_FLAG_LEFT (1 << 3)
#define ZPG_FLAG_RIGHT (1 << 4)
#define ZPG_FLAG_BELOW_LEFT (1 << 5)
#define ZPG_FLAG_BELOW (1 << 6)
#define ZPG_FLAG_BELOW_RIGHT (1 << 7)

//#define ZPG_CHAR_CODE_SOLID_BLOCK 35
#define ZPG_CHAR_CODE_SOLID_BLOCK 219

#define ZPG_GEOMETRY_TYPE_PATH 0
#define ZPG_GEOMETRY_TYPE_SOLID 1
#define ZPG_GEOMETRY_TYPE_VOID 2

#define ZPG_CELL_CATEGORY_NONE 0
#define ZPG_CELL_CATEGORY_OBJECTIVE 1
#define ZPG_CELL_CATEGORY_ENEMY 2
#define ZPG_CELL_CATEGORY_DOOR 3

#define ZPG_CELL_EMPTY 0

#define ZPG_CELL_TYPE_PATH 0
#define ZPG_CELL_TYPE_WALL 1
#define ZPG_CELL_TYPE_VOID 2

#define ZPG_CELL_TYPE_START 3
#define ZPG_CELL_TYPE_END 4
#define ZPG_CELL_TYPE_KEY 5
#define ZPG_CELL_TYPE_ENEMY 6
#define ZPG_CELL_TYPE_DOOR 71

#define ZPG_STENCIL_TYPE_EMPTY 0
#define ZPG_STENCIL_TYPE_FULL 1

#define ZPG_CELL_CHANNEL_R 0
#define ZPG_CELL_CHANNEL_G 1
#define ZPG_CELL_CHANNEL_B 2
#define ZPG_CELL_CHANNEL_A 3

#define ZPG_CAVE_GEN_SEED_CHANCE_EVEN_LOWER 0.4f
#define ZPG_CAVE_GEN_SEED_CHANCE_LOW 0.45f
#define ZPG_CAVE_GEN_SEED_CHANCE_DEFAULT 0.55f
#define ZPG_CAVE_GEN_SEED_CHANCE_HIGH 0.6f

#define ZPG_CAVE_GEN_CRITICAL_NEIGHBOURS_DEFAULT 4

#define ZPG_ERROR_NONE 0
#define ZPG_ERROR_UNKNOWN 1
#define ZPG_ERROR_UNRECOGNISED_OPTION 2

typedef int ZPGError;

//////////////////////////////////////////
// Data types
//////////////////////////////////////////

struct ZPGPoint
{
    i32 x;
    i32 y;
};

struct ZPGPointPair
{
    ZPGPoint a;
    ZPGPoint b;
};

struct ZPGRect
{
    ZPGPoint min;
    ZPGPoint max;
};

struct ZPGNeighbours
{
	i32 count;
	u32 flags;
};

#pragma pack(push, 1)
union ZPGColour
{
	u8 arr[4];
	struct
	{
		u8 r;
        u8 g;
        u8 b;
        u8 a;
	} channels;
};
#pragma pack(pop)

#define ZPG_BLACK { 0, 0, 0, 255 }
#define ZPG_WHITE { 255, 255, 255, 255 }
#define ZPG_DARK_GREY { 50, 50, 50, 255 }
#define ZPG_LIGHT_GREY { 125, 125, 125, 255 }
#define ZPG_BLUE { 0, 0, 255, 255 }
#define ZPG_GREEN { 0, 255, 0, 255 }
#define ZPG_RED { 255, 0, 0, 255 }

struct ZPGCellTypeDef
{
    u8 value;
    // Solid/floor/void
    u8 geometryType;
    // for grouping - eg none/enemy/objective/trap etc
    u8 category;
    // char to write when saving grid as text
    u8 asciChar;
    // colour to draw when saving in picture mode
    ZPGColour colour;
    // display name
    char* label;
};

struct ZPGEntity
{
    ZPGPoint pos;
    i32 degrees;
    i32 type;
    i32 tag;
};

struct ZPGGridEntityStats
{
    i32 numFloorTiles;
    i32 numObjectiveTags;
};

struct ZPGWalkCfg
{
    i32 seed;
    f32 bigRoomChance;
    i32 startX;
    i32 startY;
    i32 tilesToPlace;
    u8 typeToPaint;
    u8 bPlaceObjectives;
    u8 bStepThrough;
};

struct ZPGCellRules
{
    f32 seedChance;
    i32 starveLimit;
    i32 overpopLimit;
    i32 birthLimit;
    i32 iterations;

    u8 emptyValue;
    u8 filledValue;
};

#define ZPG_CELL_CHANNEL_0 0
#define ZPG_CELL_CHANNEL_1 1
#define ZPG_CELL_CHANNEL_2 2
#define ZPG_CELL_CHANNEL_3 3

#if 0
// TODO: This could get very messy as more functionality is piled in.
// TODO: Replace with grid stack instead
#pragma pack(push, 1)
union ZPGCell
{
    u8 arr[4];
    // Main type, stores main output values.
    struct
    {
        // channel 0
        u8 type;
        // channel 1 - heightmap. 
        u8 height;
        // channel 2 - rotation degrees divided by two
        // to fit within one byte
        u8 halfDegrees;
        // channel 3 - Extra working stencil value
        // eg recording that a cell has been visited
        u8 tag; // TODO: Move into separate working grid like ent data
    } tile;
    // Store additional data about another grid for placing entities 
    struct
    {
        u8 type;
        // immediate neighbour tile types
        u8 neighbours;
        // count of concentric rings of neighbours
        u8 rings;
    } entData;
    // styling data
    struct
    {
        u8 neighbourFlags;
        u8 sheetIndex;
    } style;
    // Treat the grid as a 32bit texture
    struct
    {
        u8 r, g, b, a;
    } colour;
};
#pragma pack(pop)
#endif

#define ZPG_IS_POS_SAFE(gridWidthI32, gridHeightI32, gridPosX, gridPosY) \
((gridPosX) >= 0 && (gridPosX) < (gridWidthI32) && (gridPosY) >= 0 && (gridPosY) < gridHeightI32)

#define ZPG_POS_TO_INDEX(gridWidthI32, gridPosX, gridPosY) \
((gridPosX) + ((gridPosY) * (gridWidthI32)))

#define ZPG_GRID_POS_SAFE(gridPtr, gridPosX, gridPosY) \
(ZPG_IS_POS_SAFE(gridPtr->width, gridPtr->height, (gridPosX), (gridPosY)))

#define ZPG_GRID_POS_TO_INDEX(gridPtr, gridPosX, gridPosY) \
(gridPosX + (gridPosY * gridPtr->width))

#define ZPG_GRID_GET(gridPtr, gridPosX, gridPosY) \
(gridPtr->cells[ZPG_POS_TO_INDEX((gridPtr->width), (gridPosX), (gridPosY))])

#define ZPG_GRID_GET_ADDR(gridPtr, gridPosX, gridPosY) \
(&gridPtr->cells[ZPG_POS_TO_INDEX(gridPtr->width, (gridPosX), (gridPosY))])

#define ZPG_GRID_SET(gridPtr, gridPosX, gridPosY, valToWrite) \
{ if ZPG_IS_POS_SAFE(gridPtr->width, gridPtr->height, gridPosX, gridPosY) \
{ gridPtr->cells[ZPG_POS_TO_INDEX(gridPtr->width, (gridPosX), (gridPosY))] = (valToWrite); }}

#define ZPG_BGRID_GET(byteGridPtr, gridPosX, gridPosY) \
byteGridPtr->cells[ZPG_POS_TO_INDEX(byteGridPtr->width, gridPosX, gridPosY)]

#define ZPG_BGRID_SET(byteGridPtr, gridPosX, gridPosY, newCellValueU8) \
byteGridPtr->cells[ZPG_POS_TO_INDEX(byteGridPtr->width, (gridPosX), (gridPosY))] = (newCellValueU8)

struct ZPGGrid
{
    i32 width;
    i32 height;
    // id: plan is to use it to identify grids in
    // user created generate scripts
    i32 id;
    // used for generating entities
    ZPGGridEntityStats stats;
    // row by row cell store
    u8* cells;
    //ZPGCell *cells;
    u8 GetValue(i32 x, i32 y)
    {
        return this->cells[ZPG_POS_TO_INDEX(this->width, x, y)];
    }

    void SetValue(i32 x, i32 y, u8 val)
    {
        this->cells[ZPG_POS_TO_INDEX(this->width, x, y)] = val;
    }

    i32 IsSafe(i32 x, i32 y)
    {
        return ZPG_IS_POS_SAFE(this->width, this->height, x, y);
    }
};

#define ZPG_MAX_GRID_STACKS 32
struct ZPGGridStack
{
    i32 numGrids;
    i32 maxGrids;
    i32 width;
    i32 height;
    ZPGGrid* grids[ZPG_MAX_GRID_STACKS];
};

#define ZPG_MAX_PREFAB_EXITS 4
struct ZPGGridPrefab
{
    char* label;
    ZPGGrid* grid;
    i32 numExits;
    ZPGPoint exits[ZPG_MAX_PREFAB_EXITS];
    ZPGPoint exitDirs[ZPG_MAX_PREFAB_EXITS];
};

// void *(__cdecl *)(size_t)
typedef void* (*zpg_allocate_fn)(size_t size);
typedef void (*zpg_free_fn)(void* ptr);
typedef void (*zpg_fatal_fn)(const char* msg);

//////////////////////////////////////////
// Functions
//////////////////////////////////////////
// API version is for external users to check the interface they are calling
ZPG_EXPORT i32 ZPG_ApiVersion();
// On error init returns a none 0 code
// TODO: Init is called automatically anyway in run functions
ZPG_EXPORT i32 ZPG_Init(zpg_allocate_fn ptrAlloc, zpg_free_fn ptrFree, zpg_fatal_fn fatal);
// Shutdown clears any still allocated memory on error returns none 0
ZPG_EXPORT i32 ZPG_Shutdown();
//ZPG_EXPORT void ZPG_RunPreset(i32 mode, char* outputPath, i32 apiFlags,
//    u8** resultPtr, i32* resultWidth, i32* resultHeight);
ZPG_EXPORT void ZPG_RunPresetCLI(
    i32 argc, char** argv,
    u8** resultPtr, i32* resultWidth, i32* resultHeight);
ZPG_EXPORT i32 ZPG_RunScript(u8* text, i32 textLength, i32 apiFlags);
ZPG_EXPORT void ZPG_PrintPrefabs();
ZPG_EXPORT void ZPG_PrintTileTypes();

#endif // ZPG_H
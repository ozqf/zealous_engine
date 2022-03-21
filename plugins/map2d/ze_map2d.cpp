#include "../ze_map2d.h"

static const char* g_map_1 =
"aabbs\n"
"0 0 -13 7 13 7\n"
"0 0 -13 -8 13 -8\n"
"0 0 -14 -8 -14 7\n"
"0 0 14 -8 14 7\n"
"0 0 -13 -5 -10 -5\n"
"0 0 10 -5 13 -5\n"
"0 0 -3 -5 3 -5\n"
"0 1 -9 -5 9 -5\n"
"\n"
;

static i32 g_bInitialised = NO;
static ZEngine g_ze;

ze_internal i32 Map2d_CountAABBsInAscii(char* txt)
{
    const i32 bufSize = 64;
    char buf[bufSize];
    char* cursor = txt;
    i32 reading = NO;
    i32 count = 0;
    while (ZStr_ReadLine(&cursor, buf, bufSize))
    {
        i32 i = ZStr_FindFirstCharMatch(buf, '\n');
        if (i != -1)
        {
            buf[i] = '\0';
        }
        if (reading)
        {
            if (buf[0] == '\0')
            {
                return count;
            }
            count++;
        }
        else
        {
            if (ZStr_Equal(buf, "aabbs"))
            {
                reading = YES;
            }
        }
    }
    return count;
}

ze_internal void Map2d_AABBFromAscii(Map2dAABB* aabb, char* txt)
{
    const i32 bufSize = 128;
    const i32 maxTokens = 32;
    char buf[bufSize];
    char* tokens[maxTokens];
    i32 numTokens = ZStr_Tokenise(txt, buf, tokens, maxTokens);
    if (numTokens != 6)
    {
        printf("\tToken count mismatch. Expected %d got %d\n", 6, numTokens);
        *aabb = {};
        return;
    }
    aabb->id = ZStr_AsciToInt32(tokens[0]);
    aabb->type = ZStr_AsciToInt32(tokens[1]);
    aabb->min.x = (f32)atof(tokens[2]);
    aabb->min.y = (f32)atof(tokens[3]);
    aabb->max.x = (f32)atof(tokens[4]);
    aabb->max.y = (f32)atof(tokens[5]);
}

ze_internal zErrorCode Map2d_FromAscii(char* txt, Map2d** result)
{
	i32 numAABBs = Map2d_CountAABBsInAscii(txt);
    printf("Map2d: Read %d AABB\n", numAABBs);
    zeSize total = sizeof(Map2d) + (sizeof(Map2dAABB) * numAABBs);
    void* start = g_ze.system.Malloc(total);
    Map2d* map = (Map2d*)start;
    map->totalBytes = total;
    map->numAABBs = 0;
    Map2dAABB* aabbs = (Map2dAABB*)((i8*)start + sizeof(Map2d));
    map->offsetAABBs = (i8*)aabbs - (i8*)map;

    // iterate file
    const i32 bufSize = 64;
    char buf[bufSize];
    char* cursor = txt;
    i32 readingMode = 0;
    i32 aabbsWritten = 0;
    while (ZStr_ReadLine(&cursor, buf, bufSize))
    {
        i32 i = ZStr_FindFirstCharMatch(buf, '\n');
        if (i != -1)
        {
            buf[i] = '\0';
        }
        if (buf[0] == '\0')
        {
            readingMode = 0;
            continue;
        }
        if (readingMode == 1)
        {
            Map2dAABB* aabb = &aabbs[map->numAABBs];
            Map2d_AABBFromAscii(aabb, buf);
            // printf("Read aabb %d: %.3f, %.3f to %.3f, %.3f\n",
            //     aabb->id, aabb->min.x, aabb->min.y, aabb->max.x, aabb->max.y);
            map->numAABBs += 1;
        }
        else
        {
            if (ZStr_Equal(buf, "aabbs"))
            {
                readingMode = 1;
            }
        }
    }

    *result = map;
	return ZE_ERROR_NONE;
}

ze_external Map2d* Map2d_ReadEmbedded()
{
    ZE_ASSERT(g_bInitialised, "Map2d not initialised")
    Map2d* map = NULL;
    zErrorCode err = Map2d_FromAscii((char*)g_map_1, &map);
    return map;
}

ze_external void Map2d_Free(Map2d* map)
{
    g_ze.system.Free(map);
}

ze_external void Map2d_Init(ZEngine engine)
{
    g_ze = engine;
    g_bInitialised = YES;
}

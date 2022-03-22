#include "../ze_map2d.h"

#define TOKENISE_STR(charPtrToTokenise) \
const i32 bufSize = 128; \
const i32 maxTokens = 32; \
char buf[bufSize]; \
char* tokens[maxTokens]; \
i32 numTokens = ZStr_Tokenise(charPtrToTokenise, buf, tokens, maxTokens);

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
"ents\n"
"spawner 100 -10 4\n"
"spawner 100 10 4\n"
"\n"
;

struct Map2dCounts
{
    i32 numAABBs;
    i32 numEnts;
    zeSize totalStringChars;
};

static i32 g_bInitialised = NO;
static ZEngine g_ze;

ze_internal Map2dCounts Map2d_CountAABBsInAscii(char* txt)
{
    const i32 bufSize = 64;
    char buf[bufSize];
    char* cursor = txt;
    i32 reading = 0;
    Map2dCounts counts = {};
    while (ZStr_ReadLine(&cursor, buf, bufSize))
    {
        i32 i = ZStr_FindFirstCharMatch(buf, '\n');
        if (i != -1)
        {
            buf[i] = '\0';
        }
        if (buf[0] == '\0')
        {
            reading = 0;
            continue;
        }
        if (reading == 1)
        {
            counts.numAABBs++;
        }
        else if (reading == 2)
        {
            counts.numEnts++;
            // read the first token as this will be a string we want to store later.
            // counts.totalStringChars += ZStr_Len(buf);
            Point2 token = ZStr_FindToken(buf, 0, ' ');
            printf("Found First token at %d to %d (%c and %c) in \"%s\"\n",
                token.x, token.y, buf[token.x], buf[token.y], buf);
            // add terminator
            counts.totalStringChars += (token.y - token.x) + 1;
        }
        else
        {
            if (ZStr_Equal(buf, "aabbs"))
            {
                reading = 1;
            }
            else if (ZStr_Equal(buf, "ents"))
            {
                reading = 2;
            }
        }
    }
    return counts;
}

ze_internal void Map2d_AABBFromAscii(Map2dAABB* aabb, char* txt)
{
    TOKENISE_STR(txt)
    if (numTokens != 6)
    {
        printf("\tToken count mismatch for aabb. Expected %d got %d\n", 6, numTokens);
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

ze_internal void Map2d_EntFromAscii(
    Map2d* map, Map2dEntity* ent, char* txt, zeSize* cursorOffset)
{
    TOKENISE_STR(txt)
    if (numTokens != 4)
    {
        printf("\tToken count mismatch for ent. Expected %d got %d\n",
            4, numTokens);
        *ent = {};
        return;
    }
    ent->typeStrOffset = 0;
    // zeSize offset = zeSize(*charsCursor - map->offsetChars);
    // printf("Ent type %s - writing to offset %zd\n", tokens[0], offset);
    // ent->typeStrOffset = zeSize(*charsCursor - map->offsetChars);
    // *charsCursor += ZStr_CopyLimited(tokens[0], *charsCursor, ZStr_Len(tokens[0]));
    ent->id = ZStr_AsciToInt32(tokens[1]);
    ent->pos.x = (f32)atof(tokens[2]);
    ent->pos.y = (f32)atof(tokens[3]);
}

/*
Header|aabbs|ents|strings
*/
ze_internal zErrorCode Map2d_FromAscii(char* txt, Map2d** result)
{
	Map2dCounts counts = Map2d_CountAABBsInAscii(txt);
    printf("Map2d: Read %d AABBs, %d ents, %zd chars\n",
        counts.numAABBs, counts.numEnts, counts.totalStringChars);
    i32 headerBytes = sizeof(Map2d);
    i32 aabbBytes = sizeof(Map2dAABB) * counts.numAABBs;
    i32 entBytes = sizeof(Map2dEntity) * counts.numEnts;
    zeSize stringBytes = counts.totalStringChars;
    // zeSize total = sizeof(Map2d) + (sizeof(Map2dAABB) * counts.numAABBs);
    zeSize total = headerBytes + aabbBytes + entBytes + stringBytes;
    void* start = g_ze.system.Malloc(total);
    Map2d* map = (Map2d*)start;
    map->totalBytes = total;

    // offsets to file sections
    map->offsetAABBs = sizeof(Map2d);
    map->offsetEnts = map->offsetAABBs + aabbBytes;
    map->offsetChars = map->offsetEnts + entBytes;

    // setup pointers to sections to write to
    // aabbs
    map->numAABBs = 0;
    Map2dAABB* aabbs = (Map2dAABB*)((i8*)start + sizeof(Map2d));

    // ents
    map->numEnts = 0;
    Map2dEntity* ents = (Map2dEntity*)(i8*)start + map->offsetEnts;

    // strings
    map->numChars = (i32)counts.totalStringChars;
    zeSize cursorOffset = (zeSize)((i8*)start + map->offsetChars);
    char* charsCursor = (char*)((i8*)start + map->offsetChars);
    cursorOffset += ZStr_CopyLimited("none", charsCursor, ZStr_Len("none"));
    // charsCursor += ZStr_CopyLimited("none", charsCursor, ZStr_Len("none"));

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
            // read aabb
            Map2dAABB* aabb = &aabbs[map->numAABBs++];
            Map2d_AABBFromAscii(aabb, buf);
            printf("\tRead aabb %d: %.3f, %.3f to %.3f, %.3f\n",
                aabb->id, aabb->min.x, aabb->min.y, aabb->max.x, aabb->max.y);
        }
        else if (readingMode == 2)
        {
            // read ent
            // TODO - WIP - broken, causing weird behaviour. most likely memory out of bounds
            // Map2dEntity* ent = &ents[map->numEnts++];
            //Map2d_EntFromAscii(map, ent, buf, &cursorOffset);
        }
        else
        {
            if (ZStr_Equal(buf, "aabbs"))
            {
                readingMode = 1;
            }
            else if (ZStr_Equal(buf, "ents"))
            {
                readingMode = 2;
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

ze_external void Map2d_DebugDump(Map2d* map)
{
    if (map == NULL) { return; }
    printf("\n=== Map2d ===\n");
    printf("Counts: AABBs %d, Ents %d, chars %d\n",
        map->numAABBs, map->numEnts, map->numChars);
    printf("Offsets: AABBs %zd, Ents %zd, chars %zd\n",
        map->offsetAABBs, map->offsetEnts, map->offsetChars);
    printf("--- AABBS (%d) ---\n", map->numAABBs);
    i8* start = (i8*)map;
    Map2dAABB* aabbs = (Map2dAABB*)(start + map->offsetAABBs);
    for (i32 i = 0; i < map->numAABBs; ++i)
    {
        Map2dAABB* aabb = &aabbs[i];
        printf("%d, type %d: %.3f, %.3f to %.3f, %.3f\n",
            aabb->id, aabb->type, aabb->min.x, aabb->min.y, aabb->max.x, aabb->max.y);
    }
    printf("--- Ents (%d) ---\n", map->numEnts);
    Map2dEntity* ents = (Map2dEntity*)start + map->offsetEnts;
    for (i32 i = 0; i < map->numEnts; ++i)
    {
        Map2dEntity* ent = &ents[i];
        char* typeStr;
        if (ent->typeStrOffset >= 0)
        {
            typeStr = (char*)start + map->offsetChars + ent->typeStrOffset;
        }
        else
        {
            typeStr = "";
        }
        
        printf("%d. Type \"%s\". Pos %.3f, %.3f\n",
            ent->id, typeStr, ent->pos.x, ent->pos.y);
    }
    printf("\n");
}

ze_external void Map2d_Init(ZEngine engine)
{
    g_ze = engine;
    g_bInitialised = YES;
}

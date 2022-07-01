#include "../ze_map2d.h"

#define READ_MODE_NONE 0
#define READ_MODE_AABBS 1
#define READ_MODE_LINES 2
#define READ_MODE_ENTS 3

#define TOKENISE_STR(charPtrToTokenise) \
const i32 bufSize = 128; \
const i32 maxTokens = 32; \
char buf[bufSize]; \
char* tokens[maxTokens]; \
i32 numTokens = ZStr_Tokenise(charPtrToTokenise, buf, tokens, maxTokens);

static const char* g_map_0 =
"aabbs\n"
"0 0 -13 7 13 7\n"      // top
"0 0 -13 -8 13 -8\n"    // bottom
"0 0 -14 -8 -14 7\n"    // left
"0 0 14 -8 14 7\n"      // right
"\n"
"lines\n"
"0 0 -9 -5 9 -5\n"      // platform
"0 0 -7 -3 7 -3\n"      // platform
"\n"
"ents\n"
"spawner 100 -10 4\n"
"spawner 101 10 4\n"
"start 102 0 -2\n"
"\n"
;

static const char* g_map_1 =
"aabbs\n"
"0 0 -13 7 13 7\n"
"0 0 -13 -8 13 -8\n"
"0 0 -14 -8 -14 7\n"
"0 0 14 -8 14 7\n"
"0 0 -13 -5 -10 -5\n"
"0 0 10 -5 13 -5\n"
"0 0 -3 -5 3 -5\n"
"\n"
"lines\n"
"0 0 -9 -5 9 -5\n"
"\n"
"ents\n"
"spawner 100 -10 4\n"
"spawner 101 10 4\n"
"start 102 0 -2\n"
"\n"
;

struct Map2dCounts
{
    i32 numAABBs;
    i32 numLines;
    i32 numEnts;
    zeSize totalStringChars;
};

static i32 g_bInitialised = NO;
static ZEngine g_ze;

ZCMD_CALLBACK(Exec_Map2dCommand)
{

}

ze_internal Map2dCounts Map2d_CountMapComponentsInAscii(char* txt)
{
    const i32 bufSize = 64;
    char buf[bufSize];
    char* cursor = txt;
    i32 reading = READ_MODE_NONE;
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
            reading = READ_MODE_NONE;
            continue;
        }
        if (reading == READ_MODE_AABBS)
        {
            counts.numAABBs++;
        }
        else if (reading == READ_MODE_LINES)
        {
            counts.numLines++;
        }
        else if (reading == READ_MODE_ENTS)
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
                reading = READ_MODE_AABBS;
            }
            else if (ZStr_Equal(buf, "lines"))
            {
                reading = READ_MODE_LINES;
            }
            else if (ZStr_Equal(buf, "ents"))
            {
                reading = READ_MODE_ENTS;
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

ze_internal void Map2d_LineFromAscii(Map2dLine* line, char* txt)
{
    TOKENISE_STR(txt)
    if (numTokens != 6)
    {
        printf("\tToken count mismatch for aabb. Expected %d got %d\n", 6, numTokens);
        *line = {};
        return;
    }
    line->id = ZStr_AsciToInt32(tokens[0]);
    line->type = ZStr_AsciToInt32(tokens[1]);
    line->a.x = (f32)atof(tokens[2]);
    line->a.y = (f32)atof(tokens[3]);
    line->b.x = (f32)atof(tokens[4]);
    line->b.y = (f32)atof(tokens[5]);
}

ze_internal char* GetCharOffsetPtr(void* origin, zeSize offset)
{
    char* ptr = (char*)origin;
    return ptr + offset;
}

// Returns offset within chars array that the string was written to
ze_internal zeSize Map2d_AppendString(Map2d* map, char* str)
{
    i32 len = ZStr_Len(str);
    char* ptr = GetCharOffsetPtr(map, map->offsetChars);
    ptr += map->offsetCursorChars;
    zeSize start = map->offsetCursorChars;
    map->offsetCursorChars += (zeSize)ZStr_CopyLimited(str, ptr, len);
    return start;
}

ze_internal void Map2d_EntFromAscii(
    Map2d* map, Map2dEntity* ent, char* txt)
{
	printf("Read Ent from \"%s\"\n", txt);
	*ent = {};
    TOKENISE_STR(txt)
    if (numTokens != 4)
    {
        printf("\tToken count mismatch for ent. Expected %d got %d\n",
            4, numTokens);
        return;
    }
    ent->id = ZStr_AsciToInt32(tokens[1]);
    ent->pos.x = (f32)atof(tokens[2]);
    ent->pos.y = (f32)atof(tokens[3]);
    ent->typeStrOffset = 0;
    ent->typeStrOffset = Map2d_AppendString(map, tokens[0]);
    printf("\tEnt read type at offset %lld\n", ent->typeStrOffset);
}

ze_internal zErrorCode Map2d_FromAscii(char* txt, Map2d** result)
{
	Map2dCounts counts = Map2d_CountMapComponentsInAscii(txt);
    printf("Map2d: Read %d AABBs, %d lines, %d ents, %zd chars\n",
        counts.numAABBs, counts.numLines, counts.numEnts, counts.totalStringChars);
    
    i32 headerBytes = sizeof(Map2d);
    i32 aabbBytes = sizeof(Map2dAABB) * counts.numAABBs;
    i32 lineBytes = sizeof(Map2dLine) * counts.numLines;
    i32 entBytes = sizeof(Map2dEntity) * counts.numEnts;
    zeSize stringPadding = 256; // additional space for 'none' type and other stuff
    zeSize stringBytes = counts.totalStringChars + stringPadding;
	
	printf("Measured map2d bytes: %d Header, %d aabbs, %d lines, %d ents, strings %d\n",
		headerBytes, aabbBytes, lineBytes, entBytes, i32(stringBytes));
    
    zeSize total =
        headerBytes
        + aabbBytes
        + lineBytes
        + entBytes
        + stringBytes;
    i8* start = (i8*)g_ze.system.Malloc(total);
    ZE_SET_ZERO(start, total);
    Map2d* map = (Map2d*)start;
    map->totalBytes = total;

    // offsets to file sections
    map->offsetAABBs = sizeof(Map2d);
    map->offsetLines = map->offsetAABBs + aabbBytes;
    map->offsetEnts = map->offsetLines + lineBytes;
    map->offsetChars = map->offsetEnts + entBytes;
    map->offsetCursorChars = 0;

    // setup pointers to sections to write to
    // aabbs
    map->numAABBs = 0;
    Map2dAABB* aabbs = (Map2dAABB*)(start + map->offsetAABBs);

    // liens
    map->numLines = 0;
    Map2dLine* lines = (Map2dLine*)(start + map->offsetLines);

    // ents
    map->numEnts = 0;
    Map2dEntity* ents = (Map2dEntity*)(start + map->offsetEnts);

    // strings
    
    map->numChars = (i32)counts.totalStringChars;
    Map2d_AppendString(map, "None");
    // zeSize cursorOffset = (zeSize)((i8*)start + map->offsetChars);
    // char* charsCursor = (char*)((i8*)start + map->offsetChars);
    // cursorOffset += ZStr_CopyLimited("none", charsCursor, ZStr_Len("none"));
    /*
    */
    // charsCursor += ZStr_CopyLimited("none", charsCursor, ZStr_Len("none"));


    // iterate file
    const i32 bufSize = 64;
    char lineBuf[bufSize];
    char* cursor = txt;
    i32 readingMode = READ_MODE_NONE;
    i32 aabbsWritten = 0;
    while (ZStr_ReadLine(&cursor, lineBuf, bufSize))
    {
        i32 i = ZStr_FindFirstCharMatch(lineBuf, '\n');
        if (i != -1)
        {
            lineBuf[i] = '\0';
        }
        if (lineBuf[0] == '\0')
        {
            readingMode = READ_MODE_NONE;
            continue;
        }
        if (readingMode == READ_MODE_AABBS)
        {
            // read aabb
            Map2dAABB* aabb = &aabbs[map->numAABBs++];
            Map2d_AABBFromAscii(aabb, lineBuf);
            // printf("\tRead aabb %d: %.3f, %.3f to %.3f, %.3f\n",
            //     aabb->id, aabb->min.x, aabb->min.y, aabb->max.x, aabb->max.y);
        }
        else if (readingMode == READ_MODE_LINES)
        {
            Map2dLine* line = &lines[map->numLines++];
            Map2d_LineFromAscii(line, lineBuf);
        }
        else if (readingMode == READ_MODE_ENTS)
        {
            // read ent
            // TODO - WIP - broken, causing weird behaviour. most likely memory out of bounds
            Map2dEntity* ent = &ents[map->numEnts++];
            Map2d_EntFromAscii(map, ent, lineBuf);
        }
        else
        {
            if (ZStr_Equal(lineBuf, "aabbs"))
            {
                readingMode = READ_MODE_AABBS;
            }
            else if (ZStr_Equal(lineBuf, "lines"))
            {
                readingMode = READ_MODE_LINES;
            }
            else if (ZStr_Equal(lineBuf, "ents"))
            {
                readingMode = READ_MODE_ENTS;
            }
        }
    }

    *result = map;
	return ZE_ERROR_NONE;
}

ze_external Map2d* Map2d_ReadEmbedded(i32 index)
{
    ZE_ASSERT(g_bInitialised, "Map2d not initialised")
    Map2d* map = NULL;
    zErrorCode err = ZE_ERROR_NONE;
    char* source = NULL;
    switch (index)
    {
        case 1:
        source = (char*)g_map_1;
        break;
        default:
        source = (char*)g_map_0;
        break;
    }
    err = Map2d_FromAscii((char*)g_map_0, &map);
    return map;
}

ze_external Map2dReader Map2d_CreateReader(Map2d* map)
{
    ZE_ASSERT(map != NULL, "Map2d is null")
    Map2dReader r = {};
    i8* start = (i8*)map;
    r.aabbs = (Map2dAABB*)(start + map->offsetAABBs);
    r.numAABBs = map->numAABBs;
    r.lines = (Map2dLine*)(start + map->offsetLines);
    r.numLines = map->numLines;
    r.ents = (Map2dEntity*)(start + map->offsetEnts);
    r.numEnts = map->numEnts;
    r.chars = (char*)(start + map->offsetChars);
    r.numChars = map->numChars;
    return r;
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
    printf("Offsets: AABBs %zd, Lines %zd, Ents %zd, chars %zd\n",
        map->offsetAABBs, map->offsetLines, map->offsetEnts, map->offsetChars);
    i8* start = (i8*)map;

    printf("--- AABBS (%d) ---\n", map->numAABBs);
    Map2dAABB* aabbs = (Map2dAABB*)(start + map->offsetAABBs);
    for (i32 i = 0; i < map->numAABBs; ++i)
    {
        Map2dAABB* aabb = &aabbs[i];
        printf("%d, type %d: %.3f, %.3f to %.3f, %.3f\n",
            aabb->id, aabb->type, aabb->min.x, aabb->min.y, aabb->max.x, aabb->max.y);
    }

    printf("--- Lines (%d) ---\n", map->numLines);
    Map2dLine* lines = (Map2dLine*)(start + map->offsetLines);
    for (i32 i = 0; i < map->numLines; ++i)
    {
        Map2dLine* line = &lines[i];
        printf("%d, type %d: %.3f, %.3f to %.3f, %.3f\n",
            line->id, line->type, line->a.x, line->a.y, line->b.x, line->b.y);
    }

    printf("--- Ents (%d) ---\n", map->numEnts);
    Map2dEntity* ents = (Map2dEntity*)(start + map->offsetEnts);
    for (i32 i = 0; i < map->numEnts; ++i)
    {
        Map2dEntity* ent = &ents[i];
        char* typeStr;
        if (ent->typeStrOffset >= 0)
        {
            typeStr = (char*)(start + map->offsetChars + ent->typeStrOffset);
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
    //
    g_ze.textCommands.RegisterCommand(
		"map2d", "Save/Load for map 2d editor, eg map2d save foo", Exec_Map2dCommand);
}

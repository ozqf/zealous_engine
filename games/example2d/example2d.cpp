#include "../../headers/zengine.h"

// for rand()
#include <stdlib.h>

#define TILE_SET_NAME "tile_set"

#define ENTITY_COUNT 4096
// #define ENTITY_COUNT 1024

#define BOUNCER_TAG 1

#define RANDF ((f32)rand() / RAND_MAX)
#define RANDF_RANGE(minValueF, maxValueF) (RANDF * (maxValueF - minValueF) + minValueF)

#define CREATE_ENT_PTR(entPtrName, drawObjPtr) \
Ent2d* entPtrName = NULL; \
if (drawObjPtr != NULL) { entPtrName = (Ent2d*)drawObjPtr->userData; }

struct Ent2d
{
    Vec3 velocity;
    f32 degrees;
    f32 rotDegreesPerSecond;
};

const i32 screenSize = 8;

ze_internal ZEngine g_engine;
ze_internal ZRTexture* g_tileAtlas;
ze_internal zeHandle g_scene;

ze_internal void BuildTileAtlas()
{
    // Create a little texture atlas
    const int tileSize = 16;
    g_tileAtlas = g_engine.assets.AllocTexture(tileSize * 4, tileSize * 4, TILE_SET_NAME);

    ZGen_FillTextureRect(g_tileAtlas, { 255, 255, 255, 255 }, { 0, 0 }, { tileSize * 4, tileSize * 4 });

    ColourU32 colours[] = {
        COLOUR_U32_BLACK,
        COLOUR_U32_WHITE,
        COLOUR_U32_GREY_LIGHT,
        COLOUR_U32_GREY,
        COLOUR_U32_GREY_DARK,
        COLOUR_U32_RED,
        COLOUR_U32_GREEN,
        COLOUR_U32_BLUE,
        COLOUR_U32_YELLOW,
        COLOUR_U32_PURPLE,
        COLOUR_U32_CYAN,
    };

    int i = 0;
    zeSize numColours = ZE_ARR_SIZE(colours, ColourU32);
    printf("Num colours: %lld\n", numColours);
    for (int y = 0; y < g_tileAtlas->height; y += 16)
    {
        for (int x = 0; x < g_tileAtlas->width; x += 16)
        {
            int index = i % numColours;
            ColourU32 colour = colours[index];
            printf("Writing %d colour %d, %d, %d\n", index, colour.r, colour.g, colour.b);
            ZGen_FillTextureRect(g_tileAtlas, colour, { x, y }, { tileSize, tileSize });
            i += 1;
        }
    }
}

ze_internal void Init()
{
    printf("2D demo init\n");
    BuildTileAtlas();

    g_scene = g_engine.scenes.AddScene(0, ENTITY_COUNT, sizeof(Ent2d));
    g_engine.scenes.SetProjectionOrtho(g_scene, screenSize);


    #if 1// create some random objects
    i32 count = ENTITY_COUNT;
    // i32 count = 1024;
    // i32 count = (ENTITY_COUNT - 16);
    for (i32 i = 0; i < count; ++i)
    {
        ZRDrawObj *mover = g_engine.scenes.AddFullTextureQuad(g_scene, TILE_SET_NAME, {1, 1}, COLOUR_U32_WHITE);
        mover->t.pos = { RANDF_RANGE(-4, 4), RANDF_RANGE(-4, 4), -2};
        mover->userTag = BOUNCER_TAG;
        mover->t.scale = { RANDF_RANGE(0.1f, 1.5f), RANDF_RANGE(0.1f, 1.5f), 1 };

        CREATE_ENT_PTR(ent, mover)
        ent->velocity.x = RANDF_RANGE(-5, 5);
        ent->velocity.y = RANDF_RANGE(-5, 5);
        ent->rotDegreesPerSecond = RANDF_RANGE(-360, 360);
        // ent->rotDegreesPerSecond = 180.f;
        i32 atlasFrame = (i32)RANDF_RANGE(0, 16);
    }
    #endif

    #if 0
    // test quads
    ZRDrawObj *obj1 = g_engine.scenes.AddFullTextureQuad(g_scene, TILE_SET_NAME, {1, 1});
    obj1->t.pos = { 0, 0, -4 };
    // fiddle with the UVs!
    obj1->data.quad.uvMin = { 0.25f, 0.25f };
    obj1->data.quad.uvMax = { 0.5f, 0.5f };

    // ZRDrawObj *obj2 = g_engine.scenes.AddFullTextureQuad(g_scene, TILE_SET_NAME, {1, 1});
    // obj2->t.pos = { -2, -2, -2 };
    // // f32 vx = ((f32)rand() / RAND_MAX) * 5;
    // // f32 vy = ((f32)rand() / RAND_MAX) * 5;
    // obj2->userTag = BOUNCER_TAG;

    // ZRDrawObj *obj3 = g_engine.scenes.AddFullTextureQuad(g_scene, TILE_SET_NAME, {1, 1});
    // obj3->t.pos = { 2, 2, -2 };
    // obj3->userTag = BOUNCER_TAG;

    // test line segments object
    ZRDrawObj* linesObj = g_engine.scenes.AddLinesObj(g_scene, 16);
    ZRDrawObjData* d = &linesObj->data;
    d->lines.bChained = YES;
    i32 i = 0;
    ZRDrawObj_AddLineVert(d, { -3, -2, 0 });
    ZRDrawObj_AddLineVert(d, { -1, -1.5f, 0 });
    ZRDrawObj_AddLineVert(d, { 1, 2, 0 });
    ZRDrawObj_AddLineVert(d, { 3, 1, 0 });
    printf("Created lines with %d verts\n", d->lines.numVerts);
    #endif
}

ze_internal void Shutdown()
{

}

ze_internal void Tick(ZEFrameTimeInfo timing)
{
    i32 numObjects = g_engine.scenes.GetObjectCount(g_scene);
    // printf("Tick with %d objects\n", numObjects);
    for (i32 i = 0; i < numObjects; ++i)
    {
        ZRDrawObj* obj = (ZRDrawObj*)g_engine.scenes.GetObjectByIndex(g_scene, i);
        if (obj == NULL) { continue; }
        if (obj->userTag != BOUNCER_TAG) { continue; }

        CREATE_ENT_PTR(ent, obj)

        // spin
        ent->degrees += ent->rotDegreesPerSecond * (f32)timing.interval;
        // ent->degrees = 45.f;
        f32 radians = ent->degrees * DEG2RAD;
        // printf("Ent rot %.3f\n", radians * RAD2DEG);

        // M3x3_RotateByAxis(obj->t.rotation.cells, radians, 0, 0, 1);
        M3x3_SetToIdentity(obj->t.rotation.cells);
        M3x3_RotateZ(obj->t.rotation.cells, radians);

        // move
        Vec3* pos = &obj->t.pos;

        pos->x += ent->velocity.x * (f32)timing.interval;
        pos->y += ent->velocity.y * (f32)timing.interval;

        f32 aspectRatio = g_engine.system.GetScreenInfo().aspect;
        f32 screenSizeY = screenSize;
        f32 screenSizeX = screenSizeY * aspectRatio;

        if (pos->x > screenSizeX) { pos->x = -screenSizeX; }
        if (pos->x < -screenSizeX) { pos->x = screenSizeX; }

        if (pos->y < -screenSizeY) { pos->y = screenSizeY; }
        if (pos->y > screenSizeY) { pos->y = -screenSizeY; }
    }
}

Z_GAME_WINDOWS_LINK_FUNCTION
{
    g_engine = engineImport;
    gameExport->Init = Init;
    gameExport->Tick = Tick;
    gameExport->Shutdown = Shutdown;
    gameExport->sentinel = ZE_SENTINEL;
    return ZE_ERROR_NONE;
}

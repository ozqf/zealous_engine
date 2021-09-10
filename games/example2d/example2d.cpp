#include "../../headers/zengine.h"

#define TILE_SET_NAME "tile_set"

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
    size_t numColours = ZE_ARR_SIZE(colours, ColourU32);
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

    g_scene = g_engine.scenes.AddScene(0, 256);
    /*
    // Bottom left
	ZGen_FillTextureRect(g_tileAtlas, COLOUR_U32_RED, { 0, 0 }, { 32, 32 });
	// Bottom right
	ZGen_FillTextureRect(g_tileAtlas, COLOUR_U32_GREEN, { 32, 0 }, { 32, 32 });
	// top left
	ZGen_FillTextureRect(g_tileAtlas, COLOUR_U32_BLUE, { 0, 32 }, { 32, 32 });
	// top right
	ZGen_FillTextureRect(g_tileAtlas, COLOUR_U32_YELLOW, { 32, 32 }, { 32, 32 });
    */
    M4x4_CREATE(prj)
    ZE_SetupOrthoProjection(prj.cells, 4, 16.f / 9.f);
    g_engine.scenes.SetProjection(g_scene, prj);

    ZRDrawObj *obj1 = g_engine.scenes.AddFullTextureQuad(g_scene, TILE_SET_NAME, {1, 1});
    obj1->t.pos = { 0, 0, -4 };
    // fiddle with the UVs!
    obj1->data.quad.uvMin = { 0.25f, 0.25f };
    obj1->data.quad.uvMax = { 0.5f, 0.5f };

    ZRDrawObj *obj2 = g_engine.scenes.AddFullTextureQuad(g_scene, TILE_SET_NAME, {1, 1});
    obj2->t.pos = { -2, -2, -2 };

    ZRDrawObj *obj3 = g_engine.scenes.AddFullTextureQuad(g_scene, TILE_SET_NAME, {1, 1});
    obj3->t.pos = { 2, 2, -2 };
}

ze_internal void Shutdown()
{

}

ze_internal void Tick(ZEFrameTimeInfo timing)
{

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

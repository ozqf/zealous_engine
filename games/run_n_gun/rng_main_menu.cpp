#include "rng_internal.h"

struct TextMenu
{
	char* header;
	char** items;
	i32 itemCount;
	i32 itemMax;
	i32 index;
	char* buf;
	i32 bufSize;
	zeHandle drawId;
};

ze_internal ZEngine g_engine;
ze_internal zeHandle g_menuScene;

#define MAIN_MENU_BUF_SIZE 1024
ze_internal char* g_mainHeader = "---MAIN MENU---\n W/S - Navigate\nSpace - Select\n";
ze_internal char* g_mainItems[16];
ze_internal i32 g_mainCount = 0;
ze_internal i32 g_mainIndex = -1;
ze_internal char g_mainBuf[MAIN_MENU_BUF_SIZE];
ze_internal zeHandle g_mainTxtDrawId;

ze_internal TextMenu g_mainMenu;

ze_internal void AllocTextMenu(TextMenu* menu, i32 maxItems, i32 textBufferSize)
{
	menu->items = (char**)g_engine.system.Malloc(sizeof(char*) * maxItems);
	menu->itemCount = 0;
	menu->itemMax = maxItems;
	menu->index = -1;
	menu->buf = (char*)g_engine.system.Malloc(sizeof(char) * textBufferSize);
	menu->bufSize = textBufferSize;
	menu->drawId = g_engine.scenes.AddObject(g_menuScene)->id;
}

ze_internal void AddMenuItem(TextMenu* menu, char* txt)
{
	i32 newIndex = menu->itemCount++;
	if (menu->index < 0)
	{
		menu->index = 0;
	}
	menu->items[newIndex] = txt;
}

ze_internal void AddMainItem(char* txt)
{
	i32 newIndex = g_mainCount++;
	if (g_mainIndex < 0)
	{
		g_mainIndex = 0;
	}
	g_mainItems[newIndex] = txt;
}

ze_internal void RefreshMainTextMenu()
{
	char* end = g_mainBuf + MAIN_MENU_BUF_SIZE;
	char* write = g_mainBuf;
	write += sprintf_s(write, end - write, "%s\n", g_mainHeader);
	for (i32 i = 0; i < g_mainCount; ++i)
	{
		if (g_mainIndex == i)
		{
			write += sprintf_s(write, end - write, "> ");
		}
		write += sprintf_s(write, end - write, "%s\n", g_mainItems[i]);
	}

	i32 charSettextureId = g_engine.assets.GetTexByName(
        FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME)->header.id;
	ZRDrawObj* txt = g_engine.scenes.GetObject(g_menuScene, g_mainTxtDrawId);
	txt->data.SetAsText(
		g_mainBuf,
		charSettextureId,
		COLOUR_U32_BLACK,
		COLOUR_U32_WHITE,
		0);
	txt->t.pos.y = 3.f;
	txt->t.scale = { 0.2f, 0.2f, 0.2f };
}

ze_internal i32 MoveInMenu(i32 currentIndex,i32 dir, i32 numItems)
{
	currentIndex += dir;
	if (currentIndex >= numItems)
	{
		currentIndex = 0;
	}
	if (currentIndex < 0)
	{
		currentIndex = numItems - 1;
	}
	return currentIndex;
}

ze_external void Menu_Show(int pageOverride)
{
	RNGPRINT("Show menu\n");
	u32 sceneFlags = g_engine.scenes.GetSceneFlags(g_menuScene);
	sceneFlags &= ~ZSCENE_FLAG_NO_DRAW;
	g_engine.scenes.SetSceneFlags(g_menuScene, sceneFlags);
}

ze_external void Menu_Hide()
{
	RNGPRINT("Hide menu\n");
	u32 sceneFlags = g_engine.scenes.GetSceneFlags(g_menuScene);
	sceneFlags |= ZSCENE_FLAG_NO_DRAW;
	g_engine.scenes.SetSceneFlags(g_menuScene, sceneFlags);
}

ze_external void Menu_Tick(ZEFrameTimeInfo timing)
{
	i32 bDirty = NO;
	if (g_engine.input.HasActionToggledOn(ACTION_MENU, timing.frameNumber))
	{
		App_SetApplicationState(APPLICATION_STATE_GAME);
		return;
	}

	if (g_engine.input.HasActionToggledOn("menu_down", timing.frameNumber))
	{
		g_mainIndex = MoveInMenu(g_mainIndex, 1, g_mainCount);
		bDirty = YES;
	}

	if (g_engine.input.HasActionToggledOn("menu_up", timing.frameNumber))
	{
		g_mainIndex = MoveInMenu(g_mainIndex, -1, g_mainCount);
		bDirty = YES;
	}

	if (g_engine.input.HasActionToggledOn("menu_select", timing.frameNumber))
	{
		bDirty = YES;
		char* item = g_mainItems[g_mainIndex];
		RNGPRINT("Menu select %d: %s\n", g_mainIndex, item);
		if (ZStr_Compare(item, "QUIT") == 0)
		{
			g_engine.textCommands.QueueCommand("quit");
		}
		else if (ZStr_Compare(item, "START") == 0)
		{
			g_engine.textCommands.QueueCommand("map e1m1");
		}
	}

	if (bDirty)
	{
		RefreshMainTextMenu();
	}
}

ze_external void Menu_Init(ZEngine engine)
{
	RNGPRINT("Init menus\n");
	g_engine = engine;
	g_menuScene = g_engine.scenes.AddScene(SCENE_ORDER_MENU, 1024, 0);
	M4x4_CREATE(uiPrj)
    ZE_SetupOrthoProjection(uiPrj.cells, 8, 16.f / 9.f);
    g_engine.scenes.SetProjection(g_menuScene, uiPrj);

	ZRDrawObj* txt = g_engine.scenes.AddObject(g_menuScene);
	g_mainTxtDrawId = txt->id;
	
	g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "menu_up");
	g_engine.input.AddAction(Z_INPUT_CODE_S , Z_INPUT_CODE_NULL, "menu_down");
	g_engine.input.AddAction(Z_INPUT_CODE_SPACE, Z_INPUT_CODE_NULL, "menu_select");

	AllocTextMenu(&g_mainMenu, 16, 256);
	AddMenuItem(&g_mainMenu, "START");
	AddMenuItem(&g_mainMenu, "OPTIONS");
	AddMenuItem(&g_mainMenu, "QUIT");

	AddMainItem("START");
	AddMainItem("OPTIONS");
	AddMainItem("QUIT");
	RefreshMainTextMenu();

	Menu_Hide();
}

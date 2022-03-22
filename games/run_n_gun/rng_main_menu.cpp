#include "rng_internal.h"

struct TextMenu
{
	char* header;
	char** itemLabels;
	char** itemTags;
	i32 itemCount;
	i32 itemMax;
	i32 index;
	char* buf;
	i32 bufSize;
	zeHandle drawId;
};

#define MAIN_MENU_BUF_SIZE 1024
#define MAIN_MENU_HEADER "---MAIN MENU---\n W/S - Navigate\nSpace - Select\n";

ze_internal ZEngine g_engine;
ze_internal zeHandle g_menuScene;
ze_internal TextMenu g_mainMenu;

ze_internal void AllocTextMenu(TextMenu* menu, i32 maxItems, i32 textBufferSize)
{
	menu->itemLabels = (char**)g_engine.system.Malloc(sizeof(char*) * maxItems);
	menu->itemTags = (char**)g_engine.system.Malloc(sizeof(char*) * maxItems);
	menu->itemCount = 0;
	menu->itemMax = maxItems;
	menu->index = -1;
	menu->buf = (char*)g_engine.system.Malloc(sizeof(char) * textBufferSize);
	menu->bufSize = textBufferSize;
	menu->drawId = g_engine.scenes.AddObject(g_menuScene)->id;
}

ze_internal void AddMenuItem(TextMenu* menu, char* txt, char* tag)
{
	i32 newIndex = menu->itemCount++;
	if (menu->index < 0)
	{
		menu->index = 0;
	}
	menu->itemLabels[newIndex] = txt;
	menu->itemTags[newIndex] = tag;
}

ze_internal void RefreshMenu(TextMenu* menu)
{
	char* end = menu->buf + MAIN_MENU_BUF_SIZE;
	char* write = menu->buf;
	write += sprintf_s(write, end - write, "%s\n", menu->header);
	for (i32 i = 0; i < menu->itemCount; ++i)
	{
		if (menu->index == i)
		{
			write += sprintf_s(write, end - write, "> ");
		}
		write += sprintf_s(write, end - write, "%s", menu->itemLabels[i]);
		if (menu->index == i)
		{
			write += sprintf_s(write, end - write, " <\n");
		}
		else
		{
			write += sprintf_s(write, end - write, "\n");
		}
	}

	i32 charSettextureId = g_engine.assets.GetTexByName(
        FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME)->header.id;
	ZRDrawObj* txt = g_engine.scenes.GetObject(g_menuScene, menu->drawId);
	txt->data.SetAsText(
		menu->buf,
		charSettextureId,
		COLOUR_U32_WHITE,
		COLOUR_U32_BLACK,
		ZR_ALIGNMENT_CENTRE);
	// txt->t.pos.y = 3.f;
	txt->t.scale = { 0.2f, 0.2f, 0.2f };
}

/*ze_internal i32 MoveInMenu2(i32 currentIndex,i32 dir, i32 numItems)
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
}*/

ze_internal void MoveInMenu(TextMenu* menu,i32 dir)
{
	menu->index += dir;
	if (menu->index >= menu->itemCount)
	{
		menu->index = 0;
	}
	if (menu->index < 0)
	{
		menu->index = menu->itemCount - 1;
	}
}

ze_internal char* GetMenuTag(TextMenu* menu)
{
	if (menu->index < 0)
	{
		return "";
	}
	return menu->itemTags[menu->index];
}

ze_external void Menu_Show(int pageOverride)
{
	u32 sceneFlags = g_engine.scenes.GetSceneFlags(g_menuScene);
	sceneFlags &= ~ZSCENE_FLAG_NO_DRAW;
	g_engine.scenes.SetSceneFlags(g_menuScene, sceneFlags);
}

ze_external void Menu_Hide()
{
	u32 sceneFlags = g_engine.scenes.GetSceneFlags(g_menuScene);
	sceneFlags |= ZSCENE_FLAG_NO_DRAW;
	g_engine.scenes.SetSceneFlags(g_menuScene, sceneFlags);
}

ze_external void Menu_Tick(ZEFrameTimeInfo timing)
{
	i32 bDirty = NO;
	if (g_engine.input.HasActionToggledOn(ACTION_MENU, timing.frameNumber))
	{
		g_engine.textCommands.QueueCommand(RNG_CMD_MENU_OFF);
		return;
	}

	TextMenu* menu = &g_mainMenu;
	if (g_engine.input.HasActionToggledOn("menu_down", timing.frameNumber))
	{
		MoveInMenu(menu, 1);
		bDirty = YES;
	}

	if (g_engine.input.HasActionToggledOn("menu_up", timing.frameNumber))
	{
		MoveInMenu(menu, -1);
		bDirty = YES;
	}

	if (g_engine.input.HasActionToggledOn("menu_select", timing.frameNumber))
	{
		bDirty = YES;
		char* item = GetMenuTag(menu);
		if (ZStr_Compare(item, "quit") == 0)
		{
			g_engine.textCommands.QueueCommand("quit");
		}
		else if (ZStr_Compare(item, "toggle_mode") == 0)
		{
			RNGPRINT("Toggle game/editor mode\n");
			g_engine.textCommands.QueueCommand(RNG_CMD_APP_TOGGLE);
		}
		else if (ZStr_Compare(item, "start") == 0)
		{
			g_engine.textCommands.QueueCommand("map e1m1");
		}
	}

	if (bDirty)
	{
		RefreshMenu(&g_mainMenu);
	}
}

ze_external void Menu_Init(ZEngine engine)
{
	RNGPRINT("Init menus\n");
	g_engine = engine;
	g_menuScene = g_engine.scenes.AddScene(SCENE_ORDER_MENU, 1024, 0);
	g_engine.scenes.SetProjectionOrtho(g_menuScene, 8);
	
	g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_NULL, "menu_up");
	g_engine.input.AddAction(Z_INPUT_CODE_S , Z_INPUT_CODE_NULL, "menu_down");
	g_engine.input.AddAction(Z_INPUT_CODE_SPACE, Z_INPUT_CODE_NULL, "menu_select");

	AllocTextMenu(&g_mainMenu, 16, MAIN_MENU_BUF_SIZE);
	g_mainMenu.header = MAIN_MENU_HEADER;
	AddMenuItem(&g_mainMenu, "START", "start");
	AddMenuItem(&g_mainMenu, "GAME/EDITOR", "toggle_mode");
	AddMenuItem(&g_mainMenu, "QUIT", "quit");
	RefreshMenu(&g_mainMenu);

	Menu_Hide();
}

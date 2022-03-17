#include "rng_internal.h"

const i32 screenSize = 8;
ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene;
ze_internal zeHandle g_uiScene;
ze_internal zeHandle g_debugTextObj;
ze_internal zeHandle g_playerId;
ze_internal zeHandle g_playerGunId;
ze_internal zeHandle g_cursorId;
ze_internal Vec2 g_mousePos;
ze_internal Vec2 g_mouseWorldPos;
ze_internal i32 g_platformTexture;

ze_internal i32 g_applicationState = -1;
ze_internal i32 g_bAppMenuOpen = NO;
ze_internal i32 g_gameState = GAME_STATE_PLAYING;

ze_internal void App_SetAppState(int newState);

ZCMD_CALLBACK(Exec_MapCommand)
{
	if (numTokens < 2)
	{
		RNGPRINT("Map: must specify a map name!\n");
		return;
	}
	RNGPRINT("New map: %s\n", tokens[1]);
	g_gameState = GAME_STATE_PLAYING;
	App_SetAppState(APP_STATE_GAME);
	Sim_StartNewGame(tokens[1]);
}

ZCMD_CALLBACK(Exec_RNGCommand)
{
	if (numTokens < 2)
	{
		RNGPRINT("Missing second parameter");
		return;
	}
	if (ZStr_Equal(fullString, RNG_CMD_GAME))
	{
		App_SetAppState(APP_STATE_GAME);
	}
	else if (ZStr_Equal(fullString, RNG_CMD_EDITOR))
	{
		App_SetAppState(APP_STATE_EDITOR);
	}
	else if (ZStr_Equal(fullString, RNG_CMD_MENU_ON))
	{
		g_bAppMenuOpen = YES;
		Menu_Show(-1);
	}
	else if (ZStr_Equal(fullString, RNG_CMD_MENU_OFF))
	{
		g_bAppMenuOpen = NO;
		Menu_Hide();
	}
	else if (ZStr_Equal(fullString, RNG_CMD_APP_TOGGLE))
	{
		g_applicationState == APP_STATE_GAME
			? App_SetAppState(APP_STATE_EDITOR)
			: App_SetAppState(APP_STATE_GAME);
	}
	else
	{
		RNGPRINT("Unknown rng parameter %s\n", tokens[1]);
	}
}

ze_internal void BuildGrid()
{
	i32 gridLines = 24;
	i32 gridVertCount = gridLines * gridLines * 3;
	ZRMeshAsset* grid = g_engine.assets.AllocEmptyMesh("grid", gridVertCount);
	const f32 lineSize = 0.02f;

	// vertical
	Vec3 bl = { -lineSize, -8, 0 };
	Vec3 br = { lineSize, -8, 0 }; 
	Vec3 tl = { -lineSize, 8, 0 }; 
	Vec3 tr = { lineSize, 8, 0 };
	Vec3 n = { 0, 0, -1 };
	Vec3 origin = { -12.f, 0.f };
	for (i32 i = 0; i < gridLines; ++i)
	{
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(br, origin), Vec3_Add(tr, origin), {}, {}, {}, n, n, n);
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(tr, origin), Vec3_Add(tl, origin), {}, {}, {}, n, n, n);
		origin.x += 1.f;
	}
	// horizontal
	bl = { -12, -lineSize, 0 };
	br = { 12, -lineSize, 0 }; 
	tl = { -12, lineSize, 0 }; 
	tr = { 12, lineSize, 0 };
	origin = { 0.f, -12.f };
	for (i32 i = 0; i < gridLines; ++i)
	{
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(br, origin), Vec3_Add(tr, origin), {}, {}, {}, n, n, n);
		grid->data.AddTri(Vec3_Add(bl, origin), Vec3_Add(tr, origin), Vec3_Add(tl, origin), {}, {}, {}, n, n, n);
		origin.y += 1.f;
	}
	
	ZRDrawObj* gridObj = g_engine.scenes.AddObject(g_scene);
	Transform_SetToIdentity(&gridObj->t);
	i32 matId = g_engine.assets.GetMaterialByName(FALLBACK_MATERIAL_NAME)->header.id;
	gridObj->data.SetAsMesh(grid->header.id, matId);
	RNGPRINT("Grid draw obj Id %d, mat Id %d\n", gridObj->id, matId);
}

internal void Init()
{
	// init physics plugin
	ZPhysicsInit(g_engine);

	TilesInit(g_engine);

	g_engine.textCommands.RegisterCommand(
		"map", "Start a new game, eg 'map e1m1'", Exec_MapCommand);
	g_engine.textCommands.RegisterCommand(
		"rng", "rng <command> - switch app states", Exec_RNGCommand);
	
	// setup scene
	g_scene = g_engine.scenes.AddScene(SCENE_ORDER_GAME, ENTITY_COUNT, sizeof(Ent2d));
	g_engine.scenes.SetClearColour(COLOUR_F32_BLACK);
	g_engine.scenes.ApplyDefaultOrthoProjection(g_scene, screenSize, 16.f / 9.f);
	// M4x4_CREATE(prj)
	// ZE_SetupOrthoProjection(prj.cells, screenSize, 16.f / 9.f);
	// g_engine.scenes.SetProjection(g_scene, prj);
	
	// setup UI scene
	g_uiScene = g_engine.scenes.AddScene(SCENE_ORDER_UI, 1024, 0);
	g_engine.scenes.ApplyDefaultOrthoProjection(g_uiScene, 8, 16.f / 9.f);
    // M4x4_CREATE(uiPrj)
    // ZE_SetupOrthoProjection(uiPrj.cells, 8, 16.f / 9.f);
    // g_engine.scenes.SetProjection(g_uiScene, uiPrj);
	
	// create debug text object in ui scene
	i32 charSettextureId = g_engine.assets.GetTexByName(
        FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME)->header.id;
	ZRDrawObj* textObj = g_engine.scenes.AddObject(g_uiScene);
	textObj->data.SetAsText("Test Text.", charSettextureId, COLOUR_U32_GREEN, COLOUR_U32_EMPTY, 0);
	textObj->t.scale = { 0.2f, 0.2f, 0.2f };
	textObj->t.pos = { -10, 7.5f, 0 };

	g_debugTextObj = textObj->id;
	
	// Create platform placeholder
	ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, TEX_PLATFORM);
	g_platformTexture = tex->header.id;
	ZGen_FillTexture(tex, COLOUR_U32_GREY);
	ZGen_FillTextureRect(tex, COLOUR_U32_EMPTY, { 1, 1 }, { 14, 14 });
	
	// cursor
	ZRTexture* cursorTex = g_engine.assets.AllocTexture(8, 8, TEX_CURSOR);
	ZGen_FillTexture(cursorTex, COLOUR_U32_GREEN);
	ZRDrawObj *cursor = g_engine.scenes.AddFullTextureQuad(
		g_scene,
		FALLBACK_TEXTURE_WHITE,
		{0.2f, 0.2f},
		COLOUR_F32_GREEN);
	g_cursorId = cursor->id;

	// BuildGrid();

	// register inputs
	g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, MOVE_LEFT);
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, MOVE_RIGHT);
	g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_SPACE, MOVE_UP);
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, MOVE_DOWN);
	g_engine.input.AddAction(Z_INPUT_CODE_LEFT_SHIFT, Z_INPUT_CODE_NULL, ACTION_SPECIAL);
	
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_1, Z_INPUT_CODE_NULL, ACTION_ATTACK_1);
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_2, Z_INPUT_CODE_NULL, ACTION_ATTACK_2);
	g_engine.input.AddAction(Z_INPUT_CODE_E, Z_INPUT_CODE_NULL, ACTION_USE);

	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_POS_X, NULL, "mouseX");
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_POS_Y, NULL, "mouseY");

	g_engine.input.AddAction(Z_INPUT_CODE_Q, Z_INPUT_CODE_NULL, ACTION_TIME_BACKWARD);
	g_engine.input.AddAction(Z_INPUT_CODE_E, Z_INPUT_CODE_NULL, ACTION_TIME_FORWARD);

	g_engine.input.AddAction(Z_INPUT_CODE_DOWN, Z_INPUT_CODE_NULL, ACTION_TIME_FAST_FORWARD);
	g_engine.input.AddAction(Z_INPUT_CODE_UP, Z_INPUT_CODE_NULL, ACTION_TIME_FAST_REWIND);

	g_engine.input.AddAction(Z_INPUT_CODE_ESCAPE, Z_INPUT_CODE_NULL, ACTION_MENU);

	// init sub-state module
	Ed_Init(g_engine);
	Sim_Init(g_engine, g_scene);
	Menu_Init(g_engine);
}

internal void Shutdown()
{

}

internal void UpdateCursor(zeHandle worldSceneId)
{
	g_mousePos.x = g_engine.input.GetActionValueNormalised("mouseX");
	g_mousePos.y = -g_engine.input.GetActionValueNormalised("mouseY");
	// mouse is range -1 to 1, scale up mouse by screen size
	// TODO: replace hard-coded aspect ratio!
	f32 aspectRatio = (16.f / 9.f);
	g_mousePos.x *= screenSize * aspectRatio;
	g_mousePos.y *= screenSize;

	// g_mousePos.x += camera.pos.x;
	// g_mousePos.y += camera.pos.y;

	ZRDrawObj *cursor = g_engine.scenes.GetObject(worldSceneId, g_cursorId);
	if (cursor == NULL) { return; }

	g_mouseWorldPos = g_mousePos;
	
	// offset by camera position
	Transform camera = g_engine.scenes.GetCamera(g_scene);
	g_mouseWorldPos.x += camera.pos.x;
	g_mouseWorldPos.y += camera.pos.y;

	// position crosshair
	cursor->t.pos.x = g_mouseWorldPos.x;
	cursor->t.pos.y = g_mouseWorldPos.y;
}

internal void UpdateDebugText()
{
	char* str = Sim_GetDebugText();
	// RNGPRINT("%s\n", str);
	ZRDrawObj* obj = g_engine.scenes.GetObject(g_uiScene, g_debugTextObj);
	if (obj == NULL)
	{
		RNGPRINT("UI text object is null\n");
		return;
	}
	obj->data.UpdateText(str);
}

internal void SetInputBit(u32* flags, u32 bit, char* actionName)
{
	if (g_engine.input.GetActionValue(actionName) > 0)
	{
		*flags |= bit;
	}
	else
	{
		*flags &= ~bit;
	}
}

ze_internal void PausedTick(ZEFrameTimeInfo timing, RNGTickInfo info)
{
	if (g_engine.input.HasActionToggledOn(ACTION_SPECIAL, timing.frameNumber))
	{
		g_gameState = GAME_STATE_PLAYING;
		Sim_ClearFutureFrames();
	}

	i32 speed = 1;
	if (g_engine.input.GetActionValue(ACTION_ATTACK_1) > 0)
	{
		speed = 0;
	}
	else if (g_engine.input.GetActionValue(ACTION_ATTACK_2) > 0)
	{
		speed = 2;
	}

	if (g_engine.input.GetActionValue(ACTION_TIME_FORWARD) > 0)
	{
		// tick
		switch (speed)
		{
			case 0:
			if (timing.frameNumber % 2 == 0)
			{
				Sim_TickForward(info, NO);
			}
			break;
			
			case 2:
			for (i32 i = 0; i < 3; ++i)
			{
				Sim_TickForward(info, NO);
			}
			break;

			default:
			Sim_TickForward(info, NO);
			break;
		}
	}
	else if (g_engine.input.GetActionValue(ACTION_TIME_BACKWARD) > 0)
	{
		// tick
		switch (speed)
		{
			case 0:
			if (timing.frameNumber % 2 == 0)
			{
				Sim_TickBackward(info);
			}
			break;
			
			case 2:
			for (i32 i = 0; i < 5; ++i)
			{
				Sim_TickBackward(info);
			}
			break;

			default:
			Sim_TickBackward(info);
			break;
		}
	}
}

ze_internal void PlayingTick(ZEFrameTimeInfo timing, RNGTickInfo info)
{
	if (g_engine.input.HasActionToggledOn(ACTION_SPECIAL, timing.frameNumber))
	{
		g_gameState = GAME_STATE_PAUSED;
		return;
	}
	#if 0
	if (g_engine.input.HasActionToggledOn(ACTION_ATTACK_2, timing.frameNumber))
	{
		RNGPRINT("Spawn Grunt\n");
		Vec2 pos = {};
		// pos.x = RANDF_RANGE(-10, 10);
		// pos.y = RANDF_RANGE(1, 5);
		pos = g_mouseWorldPos;
		// Sim_SpawnDebris(pos);
		Sim_SpawnEnemyGrunt(pos, ENT_EMPTY_ID);
	}
	#endif
	Sim_TickForward(info, YES);
}

ze_internal void App_SetAppState(int newState)
{
	if (g_applicationState == newState) { return; }
	i32 previous = g_applicationState;
	
	switch (newState)
	{
		case APP_STATE_GAME:
		RNGPRINT("App - game\n");
		g_applicationState = newState;
		Ed_Disable();
		break;

		case APP_STATE_EDITOR:
		RNGPRINT("App - editor\n");
		g_applicationState = newState;
		Ed_Enable();
		break;

		default:
		RNGPRINT("Unknown app state %d\n", newState);
		break;
	}
}

internal i32 CheckMainMenuOn(frameInt frameNumber)
{
	if (g_engine.input.HasActionToggledOn(ACTION_MENU, frameNumber))
	{
		// App_SetAppState(APP_STATE_PAUSED);
		RNGPRINT("Main Menu On\n");
		g_engine.textCommands.QueueCommand(RNG_CMD_MENU_ON);
		return YES;
	}
	return NO;
}

internal void Game_Tick(ZEFrameTimeInfo timing)
{
	if (CheckMainMenuOn(timing.frameNumber)) { return; }
	UpdateCursor(g_scene);
	
	RNGTickInfo info = {};
	SetInputBit(&info.buttons, INPUT_BIT_LEFT, MOVE_LEFT);
	SetInputBit(&info.buttons, INPUT_BIT_RIGHT, MOVE_RIGHT);
	SetInputBit(&info.buttons, INPUT_BIT_UP, MOVE_UP);
	SetInputBit(&info.buttons, INPUT_BIT_DOWN, MOVE_DOWN);
	SetInputBit(&info.buttons, INPUT_BIT_ATK_1, ACTION_ATTACK_1);
	SetInputBit(&info.buttons, INPUT_BIT_ATK_2, ACTION_ATTACK_2);

	info.cursorWorldPos = g_mouseWorldPos;
	info.cursorScreenPos = g_mousePos;
	// force sim to run at ticks of 60fps!
	// otherwise jitters in system frame timing create
	// jitters in playback.
	// info.delta = (f32)timing.interval;
	info.delta = 1.f / 60.f;

	if (Sim_GetPlayerStatus() == PLAYER_STATUS_DEAD && g_gameState == GAME_STATE_PLAYING)
	{
		g_gameState = GAME_STATE_PAUSED;
	}

	switch (g_gameState)
	{
		case GAME_STATE_PAUSED:
		PausedTick(timing, info);
		break;

		default:
		PlayingTick(timing, info);
		break;
	}

	UpdateDebugText();
}

internal void Tick(ZEFrameTimeInfo timing)
{
	if (g_bAppMenuOpen == YES)
	{
		Menu_Tick(timing);
		return;
	}
	switch (g_applicationState)
	{
		case APP_STATE_GAME:
		Game_Tick(timing);
		break;

		case APP_STATE_EDITOR:
		if (CheckMainMenuOn(timing.frameNumber)) { return; }
		Ed_Tick(timing);
		break;

		default:
		RNGPRINT("Unknown app state %d - switching to game\n", g_applicationState);
		App_SetAppState(APP_STATE_GAME);
		break;
	}
}

Z_GAME_WINDOWS_LINK_FUNCTION
{
    g_engine = engineImport;
	gameDef->targetFramerate = 60;
	gameDef->windowTitle = "Run N Gun";
	gameDef->bOverrideEscapeKey = YES;
    gameExport->Init = Init;
    gameExport->Tick = Tick;
    gameExport->Shutdown = Shutdown;
    gameExport->sentinel = ZE_SENTINEL;
    return ZE_ERROR_NONE;
}

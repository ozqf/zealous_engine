#include "rng_internal.h"

const i32 screenSize = 8;
ze_internal ZEngine g_engine;
ze_internal zeHandle g_scene = 0;
ze_internal zeHandle g_uiScene = 0;
ze_internal zeHandle g_debugTextObj = 0;
ze_internal zeHandle g_playerId = 0;
ze_internal zeHandle g_playerGunId = 0;
ze_internal zeHandle g_cursorId = 0;
ze_internal Vec2 g_mousePos;
ze_internal Vec2 g_mouseWorldPos;
ze_internal i32 g_platformTexture;
ze_internal i32 g_cursorTexture;

ze_internal i32 g_applicationState = -1;
ze_internal i32 g_bAppMenuOpen = NO;
ze_internal i32 g_gameState = GAME_STATE_PLAYING;

ze_internal void App_SetAppState(int newState);
internal void CreateCursor();

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
	
	// TODO: Cursor is created in the world scene which is wiped on map load!
	CreateCursor();
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

internal void CreateCursor()
{
	/*RNGPRINT("Create cursor\n");
	ZRDrawObj *cursor = g_engine.scenes.GetObject(g_scene, g_cursorId);
	if (cursor != NULL) { return; }
	
	// ZRTexture* cursorTex = g_engine.assets.AllocTexture(8, 8, TEX_CURSOR);
	// ZGen_FillTexture(cursorTex, COLOUR_U32_GREEN);
	cursor = g_engine.scenes.AddFullTextureQuad(
		g_scene,
		FALLBACK_TEXTURE_WHITE,
		{0.2f, 0.2f},
		COLOUR_F32_GREEN);
	g_cursorId = cursor->id;*/

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
	g_engine.scenes.SetProjectionOrtho(g_scene, 8);
	
	// setup UI scene
	g_uiScene = g_engine.scenes.AddScene(SCENE_ORDER_UI, 1024, 0);
	g_engine.scenes.SetProjectionOrtho(g_uiScene, 8);
    
	// create debug text object in ui scene
	i32 charSettextureId = g_engine.assets.GetTexByName(
        FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME)->header.id;
	ZRDrawObj* textObj = g_engine.scenes.AddObject(g_uiScene);
	textObj->data.SetAsText(
		"Test Text.",
		charSettextureId,
		COLOUR_U32_GREEN,
		COLOUR_U32_EMPTY,
		ZR_ALIGNMENT_NW);
	textObj->t.scale = { 0.2f, 0.2f, 0.2f };
	textObj->t.pos = { -10, 7.5f, 0 };

	g_debugTextObj = textObj->id;
	
	// Create platform placeholder
	ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, TEX_PLATFORM);
	g_platformTexture = tex->header.id;
	ZGen_FillTexture(tex, COLOUR_U32_GREY);
	ZGen_FillTextureRect(tex, COLOUR_U32_EMPTY, { 1, 1 }, { 14, 14 });
	
	// cursor
	CreateCursor();
	// old:
	/*ZRTexture* cursorTex = g_engine.assets.AllocTexture(8, 8, TEX_CURSOR);
	ZGen_FillTexture(cursorTex, COLOUR_U32_GREEN);
	ZRDrawObj *cursor = g_engine.scenes.AddFullTextureQuad(
		g_scene,
		FALLBACK_TEXTURE_WHITE,
		{0.2f, 0.2f},
		COLOUR_F32_GREEN);
	g_cursorId = cursor->id;*/
	
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
	g_engine.input.AddAction(Z_INPUT_CODE_F1, Z_INPUT_CODE_NULL, ACTION_TOGGLE_DEBUG);

	// init sub-state module
	Ed_Init(g_engine);
	Sim_Init(g_engine, g_scene);
	Menu_Init(g_engine);
	g_engine.textCommands.QueueCommand(RNG_CMD_MENU_ON);

	// sometimes want to start in the editor
	// App_SetAppState(APP_STATE_EDITOR);
}

internal void Shutdown()
{

}

internal void UpdateCursor()
{
	g_mousePos.x = g_engine.input.GetActionValueNormalised("mouseX");
	g_mousePos.y = -g_engine.input.GetActionValueNormalised("mouseY");
	// mouse is range -1 to 1, scale up mouse by screen size
	
	f32 aspectRatio = g_engine.system.GetScreenInfo().aspect;
	g_mousePos.x *= screenSize * aspectRatio;
	g_mousePos.y *= screenSize;
}

internal void UpdateGameCursor(zeHandle worldSceneId)
{
	// update world position for cursor
	g_mouseWorldPos = g_mousePos;
	
	// offset by camera position
	Transform camera = g_engine.scenes.GetCamera(g_scene);
	g_mouseWorldPos.x += camera.pos.x;
	g_mouseWorldPos.y += camera.pos.y;

	// update visual of cursor
	ZRDrawObj *cursor = g_engine.scenes.GetObject(worldSceneId, g_cursorId);
	if (cursor == NULL) { return; }
	printf("Cursor global Id %d, obj id %d\n", g_cursorId, cursor->id);
	
	// position crosshair
	cursor->t.pos.x = g_mouseWorldPos.x;
	cursor->t.pos.y = g_mouseWorldPos.y;
}

ze_external Vec2 App_GetCursorScreenPos()
{
	return g_mousePos;
}

internal void UpdateDebugText()
{
	char* str = "";
	if (g_applicationState == APP_STATE_GAME)
	{
		str = Sim_GetDebugText();
	}
	else if (g_applicationState == APP_STATE_EDITOR)
	{
		str = Ed_GetDebugText();
	}
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
		g_engine.scenes.SetSceneFlag(g_scene, ZSCENE_FLAG_NO_DRAW, NO);
		Ed_Disable();
		break;

		case APP_STATE_EDITOR:
		RNGPRINT("App - editor\n");
		g_applicationState = newState;
		g_engine.scenes.SetSceneFlag(g_scene, ZSCENE_FLAG_NO_DRAW, YES);
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
	UpdateGameCursor(g_scene);
	
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
}

internal void Tick(ZEFrameTimeInfo timing)
{
	UpdateCursor();
	if (g_engine.input.HasActionToggledOn(ACTION_TOGGLE_DEBUG, timing.frameNumber))
	{
		u32 flags = g_engine.scenes.GetSceneFlags(g_uiScene);
		if (IF_BIT(flags, ZSCENE_FLAG_NO_DRAW))
		{
			g_engine.scenes.SetSceneFlag(g_uiScene, ZSCENE_FLAG_NO_DRAW, 0);
		}
		else
		{
			g_engine.scenes.SetSceneFlag(g_uiScene, ZSCENE_FLAG_NO_DRAW, 1);
		}
		// flags ^= ~ZSCENE_FLAG_NO_DRAW;
		// g_engine.scenes.SetSceneFlags(g_uiScene, flags);
		
	}
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
	UpdateDebugText();
}

internal void Draw(ZRenderer renderer)
{
	RNGPRINT("Draw!\n");
	ZEBuffer buf = Buf_FromMalloc(g_engine.system.Malloc, MegaBytes(1));

	BUF_BLOCK_BEGIN_STRUCT(prjCmd, ZRDrawCmdSetCamera, (&buf), ZR_DRAW_CMD_SET_CAMERA);
	
	g_engine.system.Free(buf.start);
}

Z_GAME_WINDOWS_LINK_FUNCTION
{
    g_engine = engineImport;
	gameDef->targetFramerate = 60;
	gameDef->windowTitle = "Run N Gun";
	gameDef->flags = GAME_DEF_FLAG_OVERRIDE_ESCAPE_KEY | GAME_DEF_FLAG_MANUAL_RENDER;
    gameExport->Init = Init;
    gameExport->Tick = Tick;
    gameExport->Shutdown = Shutdown;
	gameExport->Draw = Draw;
    gameExport->sentinel = ZE_SENTINEL;
    return ZE_ERROR_NONE;
}

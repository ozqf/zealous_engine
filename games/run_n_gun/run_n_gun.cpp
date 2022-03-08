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

ze_internal i32 g_gameState = GAME_STATE_PLAYING;

/*internal void AddPlayer(Vec3 pos)
{
	// player avatar
	ZRDrawObj *player = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
	g_playerId = player->id;
	player->t.pos = pos;
	CREATE_ENT_PTR(ent, player)
	ent->type = ENT_TYPE_PLAYER;
	ZPShapeDef def = {};
	def.pos = Vec2_FromVec3(pos);
	def.size = { 1, 1, };
	def.friction = 0;
	def.resitition = 0;
	ent->bodyId = ZP_AddDynamicVolume(def);

	// gun
	ZRDrawObj *gun = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.75f, 0.1f});
	g_playerGunId = gun->id;
	gun->t.pos = pos;
	gun->t.pos.z += 0.1f;

}*/

internal void Init()
{
	// init physics plugin
	ZPhysicsInit(g_engine);

	TilesInit(g_engine);
	
	// setup scene
	g_scene = g_engine.scenes.AddScene(0, ENTITY_COUNT, sizeof(Ent2d));
	g_engine.scenes.SetClearColour(COLOUR_F32_BLACK);
	M4x4_CREATE(prj)
	ZE_SetupOrthoProjection(prj.cells, screenSize, 16.f / 9.f);
	g_engine.scenes.SetProjection(g_scene, prj);
	
	// setup UI scene
	g_uiScene = g_engine.scenes.AddScene(0, 1024, 0);
    M4x4_CREATE(uiPrj)
    ZE_SetupOrthoProjection(uiPrj.cells, 8, 16.f / 9.f);
    g_engine.scenes.SetProjection(g_uiScene, uiPrj);
	
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
	
	// init sim module
	Sim_Init(g_engine, g_scene);
	
	// Create player placeholder
	// AddPlayer({0, 2, 0});
	
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

	g_engine.input.AddAction(Z_INPUT_CODE_LEFT, Z_INPUT_CODE_NULL, ACTION_TIME_BACKWARD);
	g_engine.input.AddAction(Z_INPUT_CODE_RIGHT, Z_INPUT_CODE_NULL, ACTION_TIME_FORWARD);
}

internal void Shutdown()
{

}

/*internal void TickPlayer(float delta)
{
	ZRDrawObj *player = g_engine.scenes.GetObject(g_scene, g_playerId);
	if (player == NULL) { return; }
	CREATE_ENT_PTR(ent, player)
	if (ent == NULL) { return; }
	
	Vec2 dir = {};
	if (g_engine.input.GetActionValue(MOVE_LEFT) > 0) { dir.x -= 1; }
	if (g_engine.input.GetActionValue(MOVE_RIGHT) > 0) { dir.x += 1; }

	if (g_engine.input.GetActionValue(ACTION_USE) > 0)
	{
		ZP_Raycast({-4, -4}, {4, 4});
	}

	// grab body
	BodyState state = ZP_GetBodyState(ent->bodyId);

	// update sprite to follow body
	player->t.pos = Vec3_FromVec2(state.t.pos, player->t.pos.z);
	Transform_SetRotation(&player->t, 0, 0, state.t.radians);
	
	// apply velocity changes from input
	Vec2 v = state.velocity;
	if (dir.x != 0)
	{
		v.x += (ACCLE_FORCE * delta) * dir.x;
	}
	else
	{
		// friction and stop
		v.x *= 0.9f;
	}

	if (g_engine.input.GetActionValue(MOVE_UP) > 0)
	{
		v.y = 10.f;
	}
	// v.y += -20.f * delta;

	if (v.x > MOVE_SPEED) { v.x = MOVE_SPEED; }
	if (v.x < -MOVE_SPEED) { v.x = -MOVE_SPEED; }
	// state.velocity.y += dir.y;
	ZP_SetLinearVelocity(ent->bodyId, v);
	
	// point gun at cursor
	ZRDrawObj* gun = g_engine.scenes.GetObject(g_scene, g_playerGunId);
	ZE_ASSERT(gun != NULL, "Player gun obj not found")
	gun->t.pos = player->t.pos;
	
	Transform_SetRotation(&gun->t, 0, 0, Vec2_AngleTo(Vec2_FromVec3(gun->t.pos), g_mousePos));
}*/

internal void UpdateCursor()
{
	g_mousePos.x = g_engine.input.GetActionValueNormalised("mouseX");
	g_mousePos.y = -g_engine.input.GetActionValueNormalised("mouseY");
	// mouse is range -1 to 1, scale up mouse by screen size
	// TODO: replace hard-coded aspect ratio!
	f32 aspectRatio = (16.f / 9.f);
	g_mousePos.x *= screenSize * aspectRatio;
	g_mousePos.y *= screenSize;

	// offset by camera position
	Transform camera = g_engine.scenes.GetCamera(g_scene);
	g_mousePos.x += camera.pos.x;
	g_mousePos.y += camera.pos.y;

	ZRDrawObj *cursor = g_engine.scenes.GetObject(g_scene, g_cursorId);
	if (cursor == NULL) { return; }

	cursor->t.pos.x = g_mousePos.x;
	cursor->t.pos.y = g_mousePos.y;
	// update with scroll position when that's implemented...
	g_mouseWorldPos = g_mousePos;
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
	if (g_engine.input.GetActionValue(MOVE_RIGHT) > 0)
	{
		// tick
		Sim_TickForward(info, NO);
	}
	else if (g_engine.input.GetActionValue(MOVE_LEFT) > 0)
	{
		Sim_TickBackward(info);
	}
}

ze_internal void PlayingTick(ZEFrameTimeInfo timing, RNGTickInfo info)
{
	if (g_engine.input.HasActionToggledOn(ACTION_SPECIAL, timing.frameNumber))
	{
		g_gameState = GAME_STATE_PAUSED;
		return;
	}
	if (g_engine.input.HasActionToggledOn(ACTION_ATTACK_2, timing.frameNumber))
	{
		RNGPRINT("Spawn debris\n");
		Vec2 pos = {};
		// pos.x = RANDF_RANGE(-10, 10);
		// pos.y = RANDF_RANGE(1, 5);
		pos = g_mouseWorldPos;
		// Sim_SpawnDebris(pos);
		Sim_SpawnEnemyGrunt(pos);
	}
	Sim_TickForward(info, YES);
}

internal void Tick(ZEFrameTimeInfo timing)
{
	UpdateCursor();
	
	RNGTickInfo info = {};
	SetInputBit(&info.buttons, INPUT_BIT_LEFT, MOVE_LEFT);
	SetInputBit(&info.buttons, INPUT_BIT_RIGHT, MOVE_RIGHT);
	SetInputBit(&info.buttons, INPUT_BIT_UP, MOVE_UP);
	SetInputBit(&info.buttons, INPUT_BIT_DOWN, MOVE_DOWN);
	SetInputBit(&info.buttons, INPUT_BIT_ATK_1, ACTION_ATTACK_1);

	info.cursorWorldPos = g_mouseWorldPos;
	info.cursorScreenPos = g_mousePos;
	info.delta = (f32)timing.interval;

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
	
	// TickPlayer(dt);
	// Sim_SyncDrawObjects();

	/*i32 numObjects = g_engine.scenes.GetObjectCount(g_scene);
	for (i32 i = 0; i < numObjects; ++i)
	{
		ZRDrawObj* obj = g_engine.scenes.GetObjectByIndex(g_scene, i);
		if (obj == NULL) { continue; }
		
	}*/
}

Z_GAME_WINDOWS_LINK_FUNCTION
{
    g_engine = engineImport;
	gameDef->targetFramerate = 60;
	gameDef->windowTitle = "Run N Gun";
    gameExport->Init = Init;
    gameExport->Tick = Tick;
    gameExport->Shutdown = Shutdown;
    gameExport->sentinel = ZE_SENTINEL;
    return ZE_ERROR_NONE;
}

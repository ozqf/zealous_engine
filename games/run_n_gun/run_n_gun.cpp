#include "../../headers/zengine.h"
#include "../../plugins/ze_physics2d.h"

#define TEX_PLATFORM "platform_texture"
#define TEX_CURSOR "cursor_texture"

#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"

#define ACCLE_FORCE 100
#define MOVE_SPEED 8

#define ENT_TYPE_NONE 0
#define ENT_TYPE_SOLID 1
#define ENT_TYPE_PLAYER 2

#define ENTITY_COUNT 4096

#define RANDF ((f32)rand() / RAND_MAX)
#define RANDF_RANGE(minValueF, maxValueF) (RANDF * (maxValueF - minValueF) + minValueF)

#define CREATE_ENT_PTR(entPtrName, drawObjPtr) \
Ent2d* entPtrName = NULL; \
if (drawObjPtr != NULL) { entPtrName = (Ent2d*)drawObjPtr->userData; }

struct Ent2d
{
	i32 type;
    Vec3 velocity;
    f32 degrees;
    f32 rotDegreesPerSecond;
	zeHandle bodyId = 0;
};

const i32 screenSize = 8;
internal ZEngine g_engine;
internal zeHandle g_scene;
internal zeHandle g_playerId;
internal zeHandle g_playerGunId;
internal zeHandle g_cursorId;
internal Vec2 g_mousePos;
internal i32 g_platformTexture;

internal void AddPlatform(Vec2 pos, Vec2 size)
{
	ZRDrawObj *platform = g_engine.scenes.AddFullTextureQuad(g_scene, TEX_PLATFORM, {size.x * 0.5f, size.y * 0.5f});
	CREATE_ENT_PTR(ent, platform)
	ent->type = ENT_TYPE_SOLID;
	ent->bodyId = ZP_AddStaticVolume(pos, size);
	platform->t.pos = Vec3_FromVec2(ZP_GetBodyPosition(ent->bodyId).pos, platform->t.pos.z);
	printf("Platform %d assigned body %d\n", platform->id, ent->bodyId);
}

internal void AddPlayer(Vec3 pos)
{
	// player avatar
	ZRDrawObj *player = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
	g_playerId = player->id;
	player->t.pos = pos;
	CREATE_ENT_PTR(ent, player)
	ent->type = ENT_TYPE_SOLID;
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

	// cursor
	ZRTexture* cursorTex = g_engine.assets.AllocTexture(8, 8, TEX_CURSOR);
	ZGen_FillTexture(cursorTex, COLOUR_U32_GREEN);
	ZRDrawObj *cursor = g_engine.scenes.AddFullTextureQuad(g_scene, TEX_CURSOR, {0.2f, 0.2f});
	g_cursorId = cursor->id;
}

internal void Init()
{
	// init physics plugin
	ZPhysicsInit(g_engine);
	
	// setup scene
	g_scene = g_engine.scenes.AddScene(0, ENTITY_COUNT, sizeof(Ent2d));
	g_engine.scenes.SetClearColour(COLOUR_F32_WHITE);
	M4x4_CREATE(prj)
	ZE_SetupOrthoProjection(prj.cells, screenSize, 16.f / 9.f);
	g_engine.scenes.SetProjection(g_scene, prj);
	
	// Create player placeholder
	AddPlayer({0, 2, 0});
	
	// Create platform placeholder
	ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, TEX_PLATFORM);
	g_platformTexture = tex->header.id;
	ZGen_FillTexture(tex, COLOUR_U32_GREY);
	ZGen_FillTextureRect(tex, COLOUR_U32_EMPTY, { 1, 1 }, { 14, 14 });
	
	// add platforms
	AddPlatform({ 0, -1 }, { 8, 1 });
	AddPlatform({ 0, -4 }, { 16, 1 });

	AddPlatform({ 0, 4 }, { 16, 1 });
	
	AddPlatform({ -8, 0 }, { 1, 8 });
	AddPlatform({ 8, 0 }, { 1, 8 });
	
	// register inputs
	g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, MOVE_LEFT);
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, MOVE_RIGHT);
	g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_SPACE, MOVE_UP);
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, MOVE_DOWN);
	g_engine.input.AddAction(Z_INPUT_CODE_LEFT_SHIFT, Z_INPUT_CODE_NULL, "special");
	
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_1, Z_INPUT_CODE_NULL, "attack_1");
	g_engine.input.AddAction(Z_INPUT_CODE_E, Z_INPUT_CODE_NULL, "use");

	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_POS_X, NULL, "mouseX");
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_POS_Y, NULL, "mouseY");
}

internal void Shutdown()
{

}

internal void TickPlayer(float delta)
{
	ZRDrawObj *player = g_engine.scenes.GetObject(g_scene, g_playerId);
	if (player == NULL) { return; }
	CREATE_ENT_PTR(ent, player)
	if (ent == NULL) { return; }
	
	Vec2 dir = {};
	if (g_engine.input.GetActionValue(MOVE_LEFT) > 0) { dir.x -= 1; }
	if (g_engine.input.GetActionValue(MOVE_RIGHT) > 0) { dir.x += 1; }

	if (g_engine.input.GetActionValue("use") > 0)
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
}

internal void UpdateCursor()
{
	g_mousePos.x = g_engine.input.GetActionValueNormalised("mouseX");
	g_mousePos.y = -g_engine.input.GetActionValueNormalised("mouseY");
	// mouse is range -1 to 1, scale up mouse by screen size
	// TODO: replace hard-coded aspect ratio!
	f32 aspectRatio = (16.f / 9.f);
	g_mousePos.x *= screenSize * aspectRatio;
	g_mousePos.y *= screenSize;

	ZRDrawObj *cursor = g_engine.scenes.GetObject(g_scene, g_cursorId);
	if (cursor == NULL) { return; }

	cursor->t.pos.x = g_mousePos.x;
	cursor->t.pos.y = g_mousePos.y;
}

internal void Tick(ZEFrameTimeInfo timing)
{
	f32 dt = (f32)timing.interval;
	UpdateCursor();
	ZPhysicsTick(dt);
	TickPlayer(dt);
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

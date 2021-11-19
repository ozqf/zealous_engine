#include "../../headers/zengine.h"
#include "../../plugins/ze_physics2d.h"

#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"
#define MOVE_SPEED 4

#define ENT_TYPE_NONE 0
#define ENT_TYPE_SOLID 1
#define ENT_TYPE_PLAYER 2

#define ENTITY_COUNT 4096
#define PLATFORM_TEXTURE_NAME "platform"

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
internal i32 g_platformTexture;

internal void AddPlatform(Vec2 pos, Vec2 size)
{
	ZRDrawObj *platform = g_engine.scenes.AddFullTextureQuad(g_scene, PLATFORM_TEXTURE_NAME, {size.x * 0.5f, size.y * 0.5f});
	CREATE_ENT_PTR(ent, platform)
	ent->type = ENT_TYPE_SOLID;
	ent->bodyId = ZP_AddStaticVolume(pos, size);
	platform->t.pos = Vec3_FromVec2(ZP_GetBodyPosition(ent->bodyId).pos, platform->t.pos.z);
	printf("Platform %d assigned body %d\n", platform->id, ent->bodyId);
}

internal void AddPlayer(Vec3 pos)
{
	ZRDrawObj *player = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
	g_playerId = player->id;
	player->t.pos = pos;
	CREATE_ENT_PTR(ent, player)
	ent->type = ENT_TYPE_SOLID;
	ent->bodyId = ZP_AddDynamicVolume({pos.x, pos.y}, { 0.5f, 0.5f});
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
	ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, PLATFORM_TEXTURE_NAME);
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
	g_engine.input.AddAction(Z_INPUT_CODE_A, Z_INPUT_CODE_NULL, "move_left");
    g_engine.input.AddAction(Z_INPUT_CODE_D, Z_INPUT_CODE_NULL, "move_right");
	g_engine.input.AddAction(Z_INPUT_CODE_W, Z_INPUT_CODE_SPACE, "move_up");
    g_engine.input.AddAction(Z_INPUT_CODE_S, Z_INPUT_CODE_NULL, "move_down");
	g_engine.input.AddAction(Z_INPUT_CODE_LEFT_SHIFT, Z_INPUT_CODE_NULL, "special");
	
	g_engine.input.AddAction(Z_INPUT_CODE_MOUSE_1, Z_INPUT_CODE_NULL, "attack_1");
	g_engine.input.AddAction(Z_INPUT_CODE_E, Z_INPUT_CODE_NULL, "use");
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

	Transform2d xform = ZP_GetBodyPosition(ent->bodyId);
	Vec2 bodyPos = xform.pos;
	player->t.pos = Vec3_FromVec2(bodyPos, player->t.pos.z);
	Transform_SetRotation(&player->t, 0, 0, xform.radians);

	Vec2 dir = {};
	if (g_engine.input.GetActionValue("move_left") > 0) { dir.x -= 1; }
	if (g_engine.input.GetActionValue("move_right") > 0) { dir.x += 1; }
	
	dir.x *= 10;
	dir.y *= 10;
	// player->t.pos.x += dir.x * 10.f * delta;

	ZP_ApplyForce(ent->bodyId, dir);
	/*
	Vec2 dir = {};
	if (g_engine.input.GetActionValue("move_left") > 0) { dir.x -= 1; }
	if (g_engine.input.GetActionValue("move_right") > 0) { dir.x += 1; }
	
	player->t.pos.x += dir.x * 10.f * delta;
	*/
}

internal void Tick(ZEFrameTimeInfo timing)
{
	f32 dt = (f32)timing.interval;
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

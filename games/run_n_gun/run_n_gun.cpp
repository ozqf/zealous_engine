#include "../../headers/zengine.h"

#define MOVE_LEFT "move_left"
#define MOVE_RIGHT "move_right"
#define MOVE_UP "move_up"
#define MOVE_DOWN "move_down"
#define MOVE_SPEED 4

#define ENTITY_COUNT 4096
#define PLATFORM_TEXTURE_NAME "platform"

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

internal ZEngine g_engine;
internal zeHandle g_scene;
internal zeHandle g_playerId;
internal i32 g_platformTexture;

internal void AddPlatform(Vec2 pos, Vec2 size)
{
	ZRDrawObj *platform = g_engine.scenes.AddFullTextureQuad(g_scene, PLATFORM_TEXTURE_NAME, {size.x * 0.5f, size.y * 0.5f});
	platform->t.pos = { pos.x, pos.y, 0 };
}

internal void Init()
{
	// setup scene
	g_scene = g_engine.scenes.AddScene(0, ENTITY_COUNT, sizeof(Ent2d));
	M4x4_CREATE(prj)
	ZE_SetupOrthoProjection(prj.cells, screenSize, 16.f / 9.f);
	g_engine.scenes.SetProjection(g_scene, prj);
	
	// Create player placeholder
	ZRDrawObj *player = g_engine.scenes.AddFullTextureQuad(g_scene, FALLBACK_TEXTURE_NAME, {0.5f, 0.5f});
	g_playerId = player->id;
	player->t.pos = { 0, 0, 0 };
	
	// Create platform placeholder
	ZRTexture* tex = g_engine.assets.AllocTexture(16, 16, PLATFORM_TEXTURE_NAME);
	g_platformTexture = tex->header.id;
	ZGen_FillTexture(tex, COLOUR_U32_GREY);
	
	// add platforms
	AddPlatform({ 0, -1 }, { 8, 1 });
	
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
	
	Vec2 dir = {};
	if (g_engine.input.GetActionValue("move_left") > 0) { dir.x -= 1; }
	if (g_engine.input.GetActionValue("move_right") > 0) { dir.x += 1; }
	
	player->t.pos.x += dir.x * 10.f * delta;
}

internal void Tick(ZEFrameTimeInfo timing)
{
	f32 dt = (f32)timing.interval;
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

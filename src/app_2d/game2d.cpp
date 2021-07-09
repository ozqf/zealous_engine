/*
Simple 2D game implementation
*/
#include "game2d.h"
#include "../../headers/zr_scene.h"

internal i32 g_newId = 1;
internal ZEBlobStore g_objects = {};
internal i32 g_playerObjectId = 0;
internal ze_platform_export g_platform;
internal ZRSceneId g_gameSceneId;

struct Ent
{
	i32 id;
	i32 type;
	// transform
	Vec3 pos;
	Vec2 scale;
	f32 radians;
	// movement
	Vec2 velocity;
	// display
	i32 rendObjId;
	i32 meshId;
	i32 texId;
};

internal Ent* AddEnt(Vec3 pos, Vec2 scale, f32 radians)
{
	i32 newId = g_newId++;
	u8 *mem = g_objects.GetFreeSlot(newId);
	if (mem == NULL)
	{
		printf("No free object slot\n");
		return NULL;
	}
	
	Ent *ent = (Ent*)mem;
	*ent = {};
	ent->id = newId;
	ent->meshId = 1;
	ent->texId = 0;
	ent->pos = pos;
	ent->scale = scale;
	ent->radians = radians;

	ZRDrawObj *obj = g_platform.GetSceneManager()->AddObject(g_gameSceneId);
	if (obj != NULL)
	{
		obj->data.SetAsMesh(1, 0);
		ent->rendObjId = obj->id;
		obj->t.scale = {ent->scale.x, ent->scale.y, 1.f};
		printf("Game init obj, rendObjId: %d\n", ent->rendObjId);
	}

	return ent;
}

////////////////////////////////////////
// External
////////////////////////////////////////

extern "C" void G2d_Init(ze_platform_export platform)
{
	g_platform = platform;
	g_gameSceneId = g_platform.GetSceneManager()->CreateScene(0, 2048);
	printf("Game Scene Id: %d\n", g_gameSceneId);

	// ZRDrawObj* obj = g_platform.GetSceneManager()->AddObject(g_gameSceneId);
	// if (obj != NULL)
	// {
	// 	printf("Game init obj\n");
	// 	obj->data.SetAsMesh(1, 0);
	// }


	ZE_InitBlobStore(&g_objects, 1024, sizeof(Ent), 0);
	Ent* ent;
	
	for (i32 i = 0; i < 10; ++i)
	{
		Vec3 p =
		{
			COM_STDRandomInRange(-1, 1),
			COM_STDRandomInRange(-1, 1),
			0.1f
		};
		ent = AddEnt(p, {0.25f, 0.25f}, 0);
		ent->velocity = { COM_STDRandomInRange(-2, 2), COM_STDRandomInRange(-2, 2) };
	}	
}

extern "C" void G2d_Tick(f32 delta)
{
	i32 count = g_objects.Count();
	for (i32 i = 0; i < count; ++i)
	{
		Ent* ent = (Ent*)g_objects.GetByIndex(i);
		
		ent->pos.x += ent->velocity.x * delta;
		ent->pos.y += ent->velocity.y * delta;
		ZE_SimpleBoundaryBounce1D(&ent->pos.x, &ent->velocity.x, -1, 1);
		ZE_SimpleBoundaryBounce1D(&ent->pos.y, &ent->velocity.y, -1, 1);

		g_platform.GetSceneManager()->MoveObject(g_gameSceneId, ent->rendObjId, ent->pos);
	}
}

extern "C" void G2d_Draw(ZRViewFrame *frame)
{
	ZEBuffer *list = frame->list;
	///////////////////////////////////////////////
	// Start a new scene...
	ZRSceneFrame *scene;
	scene = ZRScene_InitInPlace(frame->list, ZR_PROJECTION_MODE_ORTHO_BASE, false);
	frame->numScenes += 1;
	// mark start of scene's objects list
	scene->params.objects = (ZRDrawObj *)list->cursor;
	Transform_SetToIdentity(&scene->params.camera);
	
	i32 count = g_objects.Count();
	for (i32 i = 0; i < count; ++i)
	{
		Ent* ent = (Ent*)g_objects.GetByIndex(i);
		
		ZRDrawObj* obj = ZRDrawObj_InitInPlace(&list->cursor);
		scene->params.numObjects += 1;
		
		obj->data.SetAsMesh(1, 0);
		obj->t.pos = ent->pos;
		obj->t.scale = {ent->scale.x, ent->scale.y, 1};
	}
	
	///////////////////////////////////////////////
	// ...Finish scene
	scene->params.numListBytes = list->cursor - (u8 *)scene->params.objects;
}


extern "C" void G2d_Shutdown()
{
	
}

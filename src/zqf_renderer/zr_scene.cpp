#include "../../headers/zqf_renderer.h"
#include "../../headers/zr_scene.h"
#include "../../headers/common/ze_blob_store.h"

struct ZRScene
{
	//ZRPlatform platform;

	// Tightly packed list of objects
	ZRSceneId id;
	i32 nextId;
	ZEBlobStore objects;
	Transform baseCamera;
	Transform baseProjection;
	// i32 bSkybox;
	// i32 bDeferred;
	// i32 bDebug;
	// i32 nextId;
	// i32 numObjects;
	// i32 maxObjects;

	// i32 projectionMode;
	// Transform camera;
};

internal ZRSceneManager g_instance = {};
internal ZRSceneId g_nextId = 1;
internal ZEBlobStore g_scenes;

internal ZRDrawObj *ZRS_AddObject(ZRSceneId id)
{
	ZRScene* scene = (ZRScene*)g_scenes.GetById(id);
	ZRDrawObj* obj = (ZRDrawObj*)scene->objects.GetFreeSlot(scene->nextId);
	ZRDrawObj_Init(obj);
	obj->id = scene->nextId;
	scene->nextId += 1;
	return obj;
}

internal void ZRS_RemoveObject(ZRScene *s)
{

}

internal void ZRS_MoveObject(ZRSceneId sceneId, ZRDrawObjId objId, Vec3 pos)
{
	ZRScene* s = (ZRScene*)g_scenes.GetById((i32)sceneId);
	ZRDrawObj *obj = (ZRDrawObj *)s->objects.GetById((i32)objId);
	obj->t.pos = pos;
}

internal void ZRS_WriteSceneForDraw(ZRViewFrame* frame, ZRScene* s)
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

	// add objects
	i32 numObjects = s->objects.Count();
	for (i32 i = 0; i < numObjects; ++i)
	{
		ZRDrawObj* source = (ZRDrawObj*)s->objects.GetByIndex(i);
		ZRDrawObj *target = ZRDrawObj_InitInPlace(&list->cursor);
		scene->params.numObjects += 1;
		*target = *source;
	}

	///////////////////////////////////////////////
	// ...Finish scene
	scene->params.numListBytes = list->cursor - (u8 *)scene->params.objects;
}

internal void ZRS_WriteScenesForDraw(ZRViewFrame *frame)
{
	// printf("Write draw scenes\n");
	i32 count = g_scenes.Count();
	for (i32 i = 0; i < count; ++i)
	{
		ZRScene* scene = (ZRScene*)g_scenes.GetByIndex(i);
		ZRS_WriteSceneForDraw(frame, scene);
	}
}

internal ZRSceneId ZRS_CreateScene(i32 order, i32 capacity)
{
	ZRSceneId id = g_nextId++;
	ZRScene* s = (ZRScene*)g_scenes.GetFreeSlot(id);
	*s = {};
	s->id = id;
	s->nextId = 1;

	ZE_InitBlobStore(&s->objects, capacity, sizeof(ZRDrawObj), 0);
	printf("Created scene %d with capacity of %d\n", id, s->objects.Capacity());
	return id;
}

ZRS_EXTERNAL ZRSceneManager* ZRS_GetSingleton()
{
	return &g_instance;
}

ZRS_EXTERNAL void ZRS_Init()
{
	printf("Init scene manager\n");
	ZE_InitBlobStore(&g_scenes, 16, sizeof(ZRScene), ZRS_INVALID_SID);
	g_instance.CreateScene = ZRS_CreateScene;
	g_instance.AddObject = ZRS_AddObject;
	g_instance.MoveObject = ZRS_MoveObject;
	g_instance.WriteForDraw = ZRS_WriteScenesForDraw;
}

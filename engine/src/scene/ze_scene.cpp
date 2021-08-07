#include "ze_scene_internal.h"

#define MAX_SCENES 16

internal zeHandle g_nextHandle = 1;
internal ZEHashTable* g_scenes = NULL;
internal ZEBuffer g_drawCommands;
internal zeHandle g_drawOrderList[MAX_SCENES];

///////////////////////////////////////////////////////////
// Scenes
///////////////////////////////////////////////////////////
internal ZRScene* GetSceneByHandle(zeHandle handle)
{
    return (ZRScene*)g_scenes->FindPointer(handle);
}

ze_external zeHandle ZScene_CreateScene(i32 order, i32 capacity)
{
    zeHandle result = g_nextHandle;
    g_nextHandle += 1;
    ZRScene* scene = (ZRScene*)Platform_Alloc(sizeof(ZRScene));
    *scene = {};
    scene->id = result;
    scene->nextId = 1;
    Transform_SetToIdentity(&scene->camera);
    Transform_SetRotationDegrees(&scene->camera, 45.f, 0, 0);
    scene->camera.pos.z = -2.f;
    ZE_SetupDefault3DProjection(scene->projection.cells, 16.f / 9.f);

    ZE_InitBlobStore(Platform_Alloc, &scene->objects, capacity, sizeof(ZRScene), 0);
    ZEHashTableData d;
    d.ptr = scene;
    g_scenes->Insert(result, d);
    printf("added scene %d - capacity %d\n", result, capacity);
    return result;
}

ze_external void ZScene_RemoveScene(zeHandle handle)
{

}

ze_external void ZScene_SetFlags(zeHandle handle, u32 flags)
{
    ZRScene *scene = GetSceneByHandle(handle);
    ZE_ASSERT(scene != NULL, "Set flags - no scene found")
    scene->flags = flags;
}

ze_external u32 ZScene_GetFlags(zeHandle handle)
{
    ZRScene *scene = GetSceneByHandle(handle);
    ZE_ASSERT(scene != NULL, "Set flags - no scene found")
    return scene->flags;
}

ze_external Transform ZScene_GetCamera(zeHandle sceneHandle)
{
    ZRScene *scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL)
    {
        TRANSFORM_CREATE(identityTransform)
        return identityTransform;
    }
    return scene->camera;
}

ze_external void ZScene_SetCamera(zeHandle sceneHandle, Transform t)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return; }
    scene->camera = t;
}

ze_external void ZScene_SetProjection(zeHandle sceneHandle, M4x4 projection)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return; }
    scene->projection = projection;
}

///////////////////////////////////////////////////////////
// Scene Objects
///////////////////////////////////////////////////////////
ze_external ZRDrawObj* ZScene_AddObject(zeHandle sceneHandle)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return NULL; }
    ZRDrawObj* obj = (ZRDrawObj*)scene->objects.GetFreeSlot(scene->nextId);
    if (obj == NULL) { return NULL; }
    *obj = {};
    obj->userTag = scene->nextId;
    scene->nextId += 1;
    return obj;
}

internal ZRDrawObj* ZScene_GetObjectById(zeHandle sceneHandle, zeHandle objectId)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return NULL; }
    ZRDrawObj *obj = (ZRDrawObj *)scene->objects.GetById(objectId);
    return obj;
}

///////////////////////////////////////////////////////////
// service
///////////////////////////////////////////////////////////

ze_external void ZScene_Draw()
{
    #if 1
    ZE_PRINTF("=== FRAME ===\n");
    ZR_ClearFrame({ 0.1f, 0.1f, 0.1f, 1});
    ZEBuffer* buf = &g_drawCommands;
    buf->Clear(NO);

    // write scene commands
    i32 len = g_scenes->m_maxKeys;
    for (i32 i = 0; i < len; ++i)
    {
        ZEHashTableKey* key = &g_scenes->m_keys[i];
        if (key->id == 0) { continue; }
        ZRScene* scene = (ZRScene*)key->data.ptr;
        if (scene->flags & ZSCENE_FLAG_NO_DRAW) { continue; }
        ZScene_WriteDrawCommands(buf, scene);
    }
    ZR_ExecuteCommands(buf);
    Platform_SubmitFrame();
    #endif

    #if 0
    ZR_DrawTest();
    Platform_SubmitFrame();
    #endif
}

ze_external ZSceneManager ZScene_RegisterFunctions()
{
    g_scenes = ZE_HashTable_Create(Platform_Alloc, MAX_SCENES, NULL);
    i32 bufSize = KiloBytes(64);
    g_drawCommands = Buf_FromMalloc(Platform_Alloc, bufSize);
    g_drawCommands.Clear(YES);
    ZScene_InitGrouping();
    ZSceneManager result = {};
    result.AddObject = ZScene_AddObject;
    result.AddScene = ZScene_CreateScene;
    result.GetCamera = ZScene_GetCamera;
    result.GetObject = ZScene_GetObjectById;
    result.SetCamera = ZScene_SetCamera;
    result.SetProjection = ZScene_SetProjection;
    return result;
}

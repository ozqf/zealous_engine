#include "ze_scene_internal.h"

internal zeHandle g_nextHandle = 1;
internal ZEHashTable* g_scenes = NULL;
internal ZEBuffer g_drawCommands;

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
    return result;
}

ze_external void ZScene_RemoveScene(zeHandle handle)
{

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

///////////////////////////////////////////////////////////
// service
///////////////////////////////////////////////////////////

ze_external void ZScene_Draw()
{
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
        ZScene_WriteDrawCommands(buf, scene);
    }

    #if 1 // execute
    ZR_ExecuteCommands(buf);
    Platform_SubmitFrame();
    #endif

    // -- test stuff --
    #if 0 // proper draw commands submission
    
    // setup camera
    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    Transform_SetToIdentity(&setCamera->camera);
    Transform_SetRotationDegrees(&setCamera->camera, 45.f, 0, 0);
    ZE_SetupDefault3DProjection(setCamera->projection.cells, 16.f / 9.f);
    
    BUF_BLOCK_BEGIN_STRUCT(spriteBatch, ZRDrawCmdSpriteBatch, buf, ZR_DRAW_CMD_SPRITE_BATCH);
    spriteBatch->textureId = ZAssets_GetTexByName("fallback_texture")->header.id;
    spriteBatch->items = (ZRSpriteBatchItem*)buf->cursor;
    spriteBatch->AddItem({ -0.5, -0.5 }, { 0.25, 0.25 }, { 0.25, 0.25 }, { 0.25, 0.25 });
    spriteBatch->AddItem({ 0.5, -0.5 }, { 0.25, 0.25 }, { 0.25, 0.75 }, { 0.25, 0.75 });
    spriteBatch->AddItem({ -0.5, 0.5 }, { 0.25, 0.25 }, { 0.75, 0.25 }, { 0.75, 0.25 });
    spriteBatch->AddItem({ 0.5, 0.5 }, { 0.25, 0.25 }, { 0.75, 0.75 }, { 0.75, 0.75 });
    spriteBatch->Finish(buf);
    ZR_ExecuteCommands(buf);
    Platform_SubmitFrame();
    #endif
    
    #if 0 // screen test
    ZR_DrawTest();
    Platform_SubmitFrame();
    #endif
}

ze_external void ZScene_Init()
{
    g_scenes = ZE_HashTable_Create(Platform_Alloc, 16, NULL);
    i32 bufSize = KiloBytes(64);
    g_drawCommands = Buf_FromMalloc(Platform_Alloc, bufSize);
    g_drawCommands.Clear(YES);

    zeHandle scene;
    ZRDrawObj* obj;

    #if 1
    scene = ZScene_CreateScene(0, 64);
    obj = ZScene_AddObject(scene);
    obj->t.pos = { -0.5f, -0.5f, 0 };
    obj->t.scale = { 0.25f, 0.25f, 0.25f };

    obj = ZScene_AddObject(scene);
    obj->t.pos = {-0.5f, 0.5f, 0};
    obj->t.scale = {0.25f, 0.25f, 0.25f};
    #endif
    #if 1
    scene = ZScene_CreateScene(0, 64);
    Transform cam = ZScene_GetCamera(scene);
    Transform_SetRotationDegrees(&cam, -45, 0, 0);
    ZScene_SetCamera(scene, cam);
    
    obj = ZScene_AddObject(scene);
    obj->t.pos = {0.5f, 0.5f, 0};
    obj->t.scale = {0.25f, 0.25f, 0.25f};
    #endif
}

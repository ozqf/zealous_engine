#include "../../internal_headers/zengine_internal.h"

/*
Visual Scene manager
Scene structs are stored in a hash table
scene structs have a blob store of the objects within them.

scenes draw in order, lowest to highest, as isolated passes

*/

struct ZRScene
{
    zeHandle id;
    // Tightly packed list of objects
    ZEBlobStore objects;
    i32 bSkybox;
    i32 bDeferred;
    i32 bDebug;
    i32 nextId;
    i32 numObjects;
    i32 maxObjects;

    i32 projectionMode;
    Transform camera;
};

internal zeHandle g_nextHandle = 1;
internal ZEHashTable* g_scenes = NULL;
internal ZEBuffer g_drawCommands;

ze_external zeHandle ZScene_AddScene(i32 order, i32 capacity)
{
    zeHandle result = g_nextHandle;
    g_nextHandle += 1;
    ZRScene* scene = (ZRScene*)Platform_Alloc(sizeof(ZRScene));
    *scene = {};
    scene-> nextId = 1;

    ZE_InitBlobStore(Platform_Alloc, &scene->objects, capacity, sizeof(ZRScene), 0);
    ZEHashTableData d;
    d.ptr = scene;
    g_scenes->Insert(result, d);
    return result;
}

ze_external ZRDrawObj* ZScene_AddObject(zeHandle sceneHandle)
{
    ZRScene* scene = (ZRScene*)g_scenes->FindPointer(sceneHandle);
    if (scene == NULL) { return NULL; }
    ZRDrawObj* obj = (ZRDrawObj*)scene->objects.GetFreeSlot(scene->nextId);
    if (obj == NULL) { return NULL; }
    *obj = {};
    obj->userTag = scene->nextId;
    scene->nextId += 1;
    return obj;
}

ze_external void ZScene_RemoveScene(zeHandle handle)
{

}

internal void WriteSceneDrawCommands(ZEBuffer* buf, ZRScene* scene)
{

}

ze_external void ZScene_Draw()
{
    
    ZR_ClearFrame({ 0.1f, 0.1f, 0.1f, 1});
    ZEBuffer* buf = &g_drawCommands;
    buf->Clear(NO);
    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    Transform_SetToIdentity(&setCamera->camera);
    Transform_SetRotationDegrees(&setCamera->camera, 45.f, 0, 0);

    ZE_SetupDefault3DProjection(setCamera->projection.cells, 16.f / 9.f);
    
    #if 1 // proper draw commands submission
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

    zeHandle scene = ZScene_AddScene(0, 1024);
    ZRDrawObj* obj = ZScene_AddObject(scene);
    obj->t.pos = { 0.5f, 0.5f, 0 };
    obj->t.scale = { 0.25f, 0.25f, 0.25f };
}

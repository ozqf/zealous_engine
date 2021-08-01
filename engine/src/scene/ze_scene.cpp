#include "../../internal_headers/zengine_internal.h"

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
    return result;
}

ze_external ZRDrawObj* ZScene_AddObject(zeHandle scene)
{
    return NULL;
}

ze_external void ZScene_RemoveScene(zeHandle handle)
{

}

ze_external void ZScene_Draw()
{
    #if 1 // proper draw commands submission
    ZR_ClearFrame({ 0.1f, 0.1f, 0.1f, 1});
    ZEBuffer* buf = &g_drawCommands;
    buf->Clear(NO);
    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    Transform_SetToIdentity(&setCamera->camera);
    Transform_SetRotationDegrees(&setCamera->camera, 45.f, 0, 0);

    ZE_SetupDefault3DProjection(setCamera->projection.cells, 16.f / 9.f);

    BUF_BLOCK_BEGIN_STRUCT(spriteBatch, ZRDrawCmdSpriteBatch, buf, ZR_DRAW_CMD_SPRITE_BATCH);
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
}

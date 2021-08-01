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
    #if 0 // proper draw commands submission
    ZEBuffer* buf = &g_drawCommands;
    // ZRDrawCmdSetCamera* setCamera = (ZRDrawCmdSetCamera*)BufBlock_Begin(
    //     buf, sizeof(ZRDrawCmdSetCamera), ZR_DRAW_CMD_SET_CAMERA);

    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    Transform_SetToIdentity(&setCamera->camera);
    Transform_SetRotationDegrees(&setCamera->camera, 45.f, 0, 0);
    // setCamera->camera = 
    // ZRDrawObj* obj;
    // ZR_DrawTest();
    ZR_ClearFrame({ 1, 0, 1, 1});
    Platform_SubmitFrame();
    #endif
    
    #if 1 // screen test
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

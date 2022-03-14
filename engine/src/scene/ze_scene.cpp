#include "ze_scene_internal.h"

#define MAX_SCENES 16

ze_internal zeHandle g_nextHandle = 1;
ze_internal ZEHashTable* g_scenes = NULL;
ze_internal ZEBuffer g_drawCommands;
ze_internal zeHandle g_drawOrderList[MAX_SCENES];
ze_internal ColourF32 g_clearColour = { 0.1f, 0.1f, 0.1f, 1 };

///////////////////////////////////////////////////////////
// Scenes
///////////////////////////////////////////////////////////
ze_internal ZRScene* GetSceneByHandle(zeHandle handle)
{
    ZRScene* result = (ZRScene*)g_scenes->FindPointer(handle);
    if (result == NULL)
    {
        ZE_BUILD_STRING(str, 256, "Scene %d was not found", handle);
        Platform_Fatal(str);
    }
    return result;
}

ze_external zeHandle ZScene_CreateScene(i32 order, i32 capacity, zeSize userDataBytesPerObject)
{
    zeHandle result = g_nextHandle;
    g_nextHandle += 1;

    // allocate scene
    ZRScene* scene = (ZRScene*)Platform_Alloc(sizeof(ZRScene));
    *scene = {};
    scene->id = result;
    scene->nextId = 1;
    scene->userStoreItemSize = userDataBytesPerObject;
    g_scenes->InsertPointer(result, scene);

    // allocate primary draw object store
    ZE_InitBlobStore(Platform_Alloc, &scene->objects, capacity, sizeof(ZRDrawObj), 0);

    // allocate user data store if required
    if (userDataBytesPerObject > 0)
    {
        ZE_InitBlobStore(
            Platform_Alloc, &scene->userStore, capacity, scene->userStoreItemSize, 0
        );
    }

    // set defaults
    Transform_SetToIdentity(&scene->camera);
    // Transform_SetRotationDegrees(&scene->camera, 45.f, 0, 0);
    scene->camera.pos.z = 4.f;
    ZE_SetupDefault3DProjection(scene->projection.cells, 16.f / 9.f);

    printf("added scene %d - capacity %d\n", result, capacity);
    return result;
}

ze_external void ZScene_RemoveScene(zeHandle handle)
{
    // TODO - err, implement this. not needed it so far...
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

internal void SetClearColour(ColourF32 colour)
{
	g_clearColour = colour;
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
    Transform_SetToIdentity(&obj->t);
    obj->id = scene->nextId;
    scene->nextId += 1;

    // acquire user data blob if necessary
    if (scene->userStoreItemSize > 0)
    {
        void* blob = scene->userStore.GetFreeSlot(obj->id);
        obj->userData = blob;
    }
    return obj;
}

ze_internal void ZScene_RemoveObject(zeHandle sceneHandle, zeHandle objectId)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return; }
    scene->objects.MarkForRemoval(objectId);
    if (scene->userStoreItemSize > 0)
    {
        scene->userStore.MarkForRemoval(objectId);
    }
}

ze_internal ZRDrawObj* ZScene_GetObjectById(zeHandle sceneHandle, zeHandle objectId)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return NULL; }
    ZRDrawObj *obj = (ZRDrawObj *)scene->objects.GetById(objectId);
    return obj;
}

ze_internal i32 ZScene_GetObjectCount(zeHandle sceneHandle)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return 0; }
    return scene->objects.m_array->m_numBlobs;
}

ze_internal ZRDrawObj* ZScene_GetObjectByIndex(zeHandle sceneHandle, i32 i)
{
    ZRScene* scene = GetSceneByHandle(sceneHandle);
    if (scene == NULL) { return NULL; }
    return (ZRDrawObj*)scene->objects.GetByIndex(i);
}

///////////////////////////////////////////////////////////
// scene object utility functions
///////////////////////////////////////////////////////////

ze_internal ZRDrawObj* ZScene_AddFullTextureQuad(
    zeHandle scene, char* textureName, Vec2 size, ColourF32 colour)
{
	if (textureName == NULL)
	{
		textureName = FALLBACK_TEXTURE_WHITE;
	}
	
    ZRDrawObj* obj = ZScene_AddObject(scene);
    obj->data.type = ZR_DRAWOBJ_TYPE_QUAD;
    ZRTexture* tex = ZAssets_GetTexByName(textureName);
    obj->data.quad.textureId = tex->header.id;
    obj->data.quad.uvMin = { 0, 0 };
    obj->data.quad.uvMax = { 1, 1 };
    obj->data.quad.offset = { };
    obj->t.scale = { size.x, size.y, 1 };
    obj->data.quad.colour = colour;
    return obj;
}

ze_internal ZRDrawObj* ZScene_AddLinesObj(zeHandle scene, i32 maxVerts)
{
    ZRDrawObj *obj = ZScene_AddObject(scene);
    obj->data.type = ZR_DRAWOBJ_TYPE_LINES;
    obj->data.lines.verts = (ZRLineVertex*)Platform_Alloc(sizeof(ZRLineVertex) * maxVerts);
    obj->data.lines.maxVerts = maxVerts;
    obj->data.lines.numVerts = 0;
    return obj;
}

ze_internal ZRDrawObj* ZScene_AddCube(zeHandle scene, char* materialName)
{
	if (materialName == NULL)
	{
		materialName = FALLBACK_CHEQUER_MATERIAL;
	}
    ZRDrawObj *obj = ZScene_AddObject(scene);
    ZRMaterial* mat = ZAssets_GetMaterialByName(materialName);
    ZRMeshAsset* mesh = ZAssets_GetMeshByName(ZE_EMBEDDED_CUBE_NAME);
    obj->data.SetAsMesh(mesh->header.id, mat->header.id);
    return obj;
}

///////////////////////////////////////////////////////////
// service
///////////////////////////////////////////////////////////

/*
TODO: scene order value is not implemented, scenes draw in order they were created
*/
ze_external void ZScene_Draw()
{
    // ZE_PRINTF("=== FRAME ===\n");
    ZR_ClearFrame(g_clearColour);
    f64 cmdStart = Platform_QueryClock();
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
    f64 cmdEnd = Platform_QueryClock();
    f64 drawStart = Platform_QueryClock();
    ZR_ExecuteCommands(buf);
    Platform_SubmitFrame();
    f64 drawEnd = Platform_QueryClock();

    // printf("Write draw time: %.3fms - submit draw time %.3fms\n",
    //     (cmdEnd - cmdStart) * 1000.f,
    //     (drawEnd - drawStart) * 1000.f);
}

ze_external void ZScene_PostFrameTick()
{
    i32 len = g_scenes->m_maxKeys;
    for (i32 i = 0; i < len; ++i)
    {
        ZEHashTableKey* key = &g_scenes->m_keys[i];
        if (key->id == 0) { continue; }
        ZRScene* scene = (ZRScene*)key->data.ptr;
        scene->objects.Truncate();
        if (scene->userStoreItemSize > 0)
        {
            scene->userStore.Truncate();
        }
    }
}

ze_external ZSceneManager ZScene_RegisterFunctions()
{
    g_scenes = ZE_HashTable_Create(Platform_Alloc, MAX_SCENES, NULL);
    i32 bufSize = KiloBytes(64);
    g_drawCommands = Buf_FromMalloc(Platform_Alloc, bufSize);
    g_drawCommands.Clear(YES);
    ZScene_InitGrouping();
    ZSceneManager result = {};
    // core
    result.AddObject = ZScene_AddObject;
    result.AddScene = ZScene_CreateScene;
    result.GetCamera = ZScene_GetCamera;
    result.GetObject = ZScene_GetObjectById;
    result.RemoveObject = ZScene_RemoveObject;
    result.GetObjectCount = ZScene_GetObjectCount;
    result.GetObjectByIndex = ZScene_GetObjectByIndex;
    
    result.SetCamera = ZScene_SetCamera;
    result.SetProjection = ZScene_SetProjection;
	result.SetClearColour = SetClearColour;
    result.GetSceneFlags = ZScene_GetFlags;
    result.SetSceneFlags = ZScene_SetFlags;

    // utility
    result.AddCube = ZScene_AddCube;
    result.AddFullTextureQuad = ZScene_AddFullTextureQuad;
    result.AddLinesObj = ZScene_AddLinesObj;
    return result;
}

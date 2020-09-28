#ifndef ZQF_DRAW_SCENE_H
#define ZQF_DRAW_SCENE_H

#include "zqf_renderer.h"
#include "../ze_common/ze_transform.h"

#if 0

struct ZRScene
{
    //ZRPlatform platform;

    // Tightly packed list of objects
    ZRDrawObj* objects;
    i32 bSkybox;
    i32 bDeferred;
    i32 bDebug;
    i32 nextId;
    i32 numObjects;
    i32 maxObjects;

    i32 projectionMode;
    Transform camera;
};

/**
 * TODO: REPLACE THIS IMPLEMENTATION
 * This scene graph is a placeholder implementation
 * Very inefficient and full of linear searches!
 */

/**
 * Draw Scene - store of render objects
 */

#define ZR_SCENE_BAD_ID 0
#define ZR_SCENE_BAD_INDEX -1

static void ZRScene_FindOverlaps(
    ZRScene* s, i32* results, i32 maxResults);

static ZRDrawObj* ZqfScene_GetFreeObject(ZRScene* s)
{
    ZE_ASSERT(s->numObjects < s->maxObjects,
        "Draw scene out of capacity")
    ZRDrawObj* obj = &s->objects[s->numObjects];
    *obj = {};
    Transform_SetToIdentity(&obj->t);
    Transform_SetToIdentity(&obj->localT);
    s->numObjects++;
    obj->id = s->nextId++;
    obj->status = ZR_DRAWOBJ_STATUS_ASSIGNED;
    return obj;
}

// TODO: Linear search...
static i32 ZRScene_IndexOf(ZRScene* s, i32 id)
{
    if (id == ZR_SCENE_BAD_ID) { return ZR_SCENE_BAD_INDEX; }
    for (i32 i = 0; i < s->numObjects; ++i)
    {
        if (s->objects[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

static ZRDrawObj* ZRScene_FindObj(ZRScene* scene, i32 id)
{
    if (id == ZR_SCENE_BAD_ID) { return NULL; }
    i32 index = ZRScene_IndexOf(scene, id);
    if (index == ZR_SCENE_BAD_INDEX) { return NULL; }
    return &scene->objects[index];
}

static void ZRScene_MarkForDelete(ZRScene* s, i32 index)
{
    ZE_ASSERT(index >= 0, "Out of bounds")
    ZE_ASSERT(index < s->numObjects, "Out of bounds")
    s->objects[index].status = ZR_DRAWOBJ_STATUS_DELETED;
}

static void ZRScene_RemoveAt(ZRScene* s, i32 index)
{
    ZE_ASSERT(index >= 0, "Out of bounds")
    ZE_ASSERT(index < s->numObjects, "Out of bounds")
    s->objects[index] = {};
    s->numObjects--;
    if (s->numObjects == 0) { return; }
    if (index == s->numObjects) { return; }
    ZRDrawObj* temp = &s->objects[s->numObjects];
    s->objects[index] = *temp;
    *temp = {};
}

static void ZRScene_Clear(ZRScene* s)
{
    s->numObjects = 0;
    s->nextId = 1;
}

static ZRScene* ZRScene_Init(ZRPlatform platform, i32 objectCapacity)
{
    i32 structSize = sizeof(ZRScene);
    i32 objectsArraySize = sizeof(ZRDrawObj) * objectCapacity;
    
    i32 total = structSize + objectsArraySize;

    u8* mem = (u8*)platform.Allocate(total);
    ZE_ASSERT(mem != NULL,
        "Failed to allocate memory for Draw Scene")
    printf("Allocated %d bytes for draw scene, max %d objects\n",
        total, objectCapacity);

    ZRScene* s = (ZRScene*)mem;
    *s = {};
    s->nextId = 1;
    s->platform = platform;
    s->objects = (ZRDrawObj*)(mem + structSize);
    s->maxObjects = objectCapacity;
    Transform_SetToIdentity(&s->camera);
    return s;
}

static void ZRScene_WriteScene2Frame(
    ZRViewFrame* header,
    ZRScene* scene, // TODO: Split this function up so that scenes are written independently
    ZEBuffer* objectListBuf,      // stores scenes and objects
    ZEBuffer* objectDataBuf       // stores data for the objects in the list eg strings
    )
{
    header->numScenes++;
    ZRSceneFrame* cmd = (ZRSceneFrame*)objectListBuf->cursor;
    objectListBuf->cursor += sizeof(ZRSceneFrame);
    cmd->sentinel = ZR_SENTINEL;
    cmd->params.camera = scene->camera;
    cmd->params.projectionMode = scene->projectionMode;
    cmd->params.bSkybox = scene->bSkybox;
    cmd->params.bDeferred = scene->bDeferred;
    cmd->params.bIsInteresting = scene->bDebug;

    u8* dataStart = objectListBuf->cursor;
    for (i32 i = 0; i < scene->numObjects; ++i)
    {
        ZRDrawObj* obj = &scene->objects[i];

        switch (obj->type)
        {
            case ZR_DRAWOBJ_TYPE_TEXT:
            {
                ZRDrawObj* clone = (ZRDrawObj*)objectListBuf->cursor;
                *clone = *obj;
                objectListBuf->cursor += sizeof(ZRDrawObj);

                // Copy text to data buffer
                char* textSrc = obj->data.text.text;
                i32 len = obj->data.text.length;
                // Record new pointer to string in cloned object
                clone->data.text.text = (char*)objectDataBuf->cursor;

                // Copy
                objectDataBuf->cursor += ZE_COPY(
                    textSrc, objectDataBuf->cursor, obj->data.text.length);
                *objectDataBuf->cursor = '\0';
                objectDataBuf->cursor++;
            } break;

            default:
            {
                // copy straight into buf
                objectListBuf->cursor +=
                    ZE_COPY(obj, objectListBuf->cursor, sizeof(ZRDrawObj));
            } break;
        }
    }
    cmd->params.numListBytes = (objectListBuf->cursor - dataStart);
    cmd->params.numObjects = scene->numObjects;
}

/**
 * Write a ZRViewFrame to draw the current game scene
 * Returns bytes written
 */
static ZRViewFrame* ZRScene_BeginViewFrame(
    ZEBuffer* objectListBuf,      // stores scenes and objects
    ZEBuffer* objectDataBuf       // stores data for the objects in the list eg strings
    )
{
    ZE_ASSERT(Buf_IsValid(objectListBuf) == YES, "Object list buffer is invalid")
    ZE_ASSERT(Buf_IsValid(objectDataBuf) == YES, "Object data buffer is invalid")
    
    objectListBuf->Clear(NO);
    objectDataBuf->Clear(NO);
    // allocate header, Write header info at end
    ZRViewFrame* header = (ZRViewFrame*)objectListBuf->cursor;
    *header = {};
    objectListBuf->cursor += sizeof(ZRViewFrame);

    return header;
}

#endif

#endif // ZQF_DRAW_SCENE_H
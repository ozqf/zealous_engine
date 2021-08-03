/*
Zealous Engine internal header
*/
#ifndef ZENGINE_INTERNAL_H
#define ZENGINE_INTERNAL_H

#include "../../headers/zengine.h"

// scenes above this are internal - eg for debugging overlays
#define ZR_INTERNAL_SCENE_START_DEPTH 0x0FFFFFFF

#define ZR_MAX_BATCH_SIZE 256

#ifndef ZE_PRINTF
// #define ZE_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__##)
#define ZE_PRINTF(fmt, ...)
#endif

// internal types
/**
 * Asset handles required to execute a draw call
 */
struct ZRMeshHandles
{
    i32 vao;
    i32 vbo;
    i32 vertexCount;
    i32 totalVBOBytes;
    // all data before this point is static mesh geometry
    i32 instanceDataOffset;
    // Capacity for instances left behind static mesh data
    i32 maxInstances;
};

struct ZRGLHandles
{
    i32 assetId;
    i32 assetType;
    union
    {
        ZRMeshHandles meshHandles;
        u32 textureHandle;
    } data;
};

/////////////////////////////////////////
// Render commands
/////////////////////////////////////////
#define ZR_DRAW_CMD_NONE 0
#define ZR_DRAW_CMD_SET_CAMERA 1
#define ZR_DRAW_CMD_SPRITE_BATCH 2
struct ZRDrawCmdSetCamera
{
    BufferBlock header;
    Transform camera;
    M4x4 projection;
};

struct ZRSpriteBatchItem
{
    Vec3 pos;
    Vec2 size;
    Vec2 uvMin;
    Vec2 uvMax;
};

struct ZRDrawCmdSpriteBatch
{
    BufferBlock header;
    i32 textureId;
    i32 numItems;
    ZRSpriteBatchItem* items;

    void AddItem(Vec3 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax)
    {
        items[numItems].pos = pos;
        items[numItems].size = size;
        items[numItems].uvMin = uvMin;
        items[numItems].uvMax = uvMax;
        numItems++;
    }

    i32 Finish(ZEBuffer* buf)
    {
        this->header.size = sizeof(ZRDrawCmdSpriteBatch) + (sizeof(ZRSpriteBatchItem) * numItems);
        buf->cursor = (i8*)this + this->header.size;
        return this->header.size;
    }
};

//////////////////////////////////
// platform
ze_external void Platform_PollEvents();
ze_external void Platform_SubmitFrame();
ze_external void *Platform_Alloc(size_t size);
ze_external void Platform_Free(void* ptr);
ze_external void Platform_DebugBreak();
ze_external ZEBuffer Platform_StageFile(char *path);

//////////////////////////////////
// game module linkup
ze_external ZGame ZGame_StubLinkup(ZEngine engine);
ze_external zErrorCode ZE_Init();

//////////////////////////////////
// debug
ze_external zErrorCode ZDebug_Init_1();
ze_external zErrorCode ZDebug_Init_2();

//////////////////////////////////
// config
ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc);
ze_external i32 ZCFG_Init(const char *cmdLine, const char **argv, const i32 argc);
ze_external i32 ZCFG_FindParamIndex(const char* shortQuery, const char* longQuery, i32 extraTokens);

//////////////////////////////////
// asset db
ze_external ZRTexture *ZAssets_GetTexByName(char *name);
ze_external ZRTexture *ZAssets_GetTexById(i32 id);

ze_external zErrorCode ZAssets_Init();
ze_external void ZAssets_PrintAll();
ze_external ZRTexture *ZAssets_AllocTex(i32 width, i32 height, char *name);
ze_external ZRMeshData *ZAssets_AllocMesh(i32 maxVerts);

ze_external zErrorCode ZEmbedded_Init();

//////////////////////////////////
// asset creation
ze_external void ZGen_Init();
ze_external void ZGen_FillTexture(ZRTexture *tex, ColourU32 colour);
ze_external void ZGen_SetPixel(
    ZRTexture *tex, ColourU32 colour, i32 x, i32 y);
ze_external void ZGen_FillTextureRect(
    ZRTexture *tex, ColourU32 colour, Point2 topLeft, Point2 size);
ze_external void ZGen_AddSriteGeoXY(
    ZRMeshData* meshData, Vec2 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax);
ze_external i32 TexGen_DecodeBW(
    u8 *source,
    const i32 sourceSize,
    ColourU32 *target,
    const i32 w,
    const i32 h,
    ColourU32 solid,
    ColourU32 empty);

//////////////////////////////////
// scene manager
ze_external void ZScene_Init();
ze_external void ZScene_Draw();
ze_external ZEngine GetEngine();

ze_external zeHandle ZScene_CreateScene(i32 order, i32 capacity);
ze_external ZRDrawObj *ZScene_AddObject(zeHandle sceneHandle);
ze_external Transform ZScene_GetCamera(zeHandle sceneHandle);
ze_external void ZScene_SetCamera(zeHandle sceneHandle, Transform t);
ze_external void ZScene_SetProjection(zeHandle sceneHandle, M4x4 projection);

ze_external i32 ZE_StartLoop();
ze_external void ZE_Shutdown();

// renderer
ze_external zErrorCode ZR_Init();
ze_external void ZR_ClearFrame(ColourF32 colour);
ze_external void ZR_ExecuteCommands(ZEBuffer* commandBuffer);
ze_external zErrorCode ZR_DrawTest();
ze_external void ZRGL_PrintHandles();

#endif // ZENGINE_INTERNAL_H

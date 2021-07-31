/*
Zealous Engine internal header
*/
#ifndef ZENGINE_INTERNAL_H
#define ZENGINE_INTERNAL_H

#include "../../headers/zengine.h"

#define ZR_MAX_BATCH_SIZE 256

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

ze_external void Platform_PollEvents();
ze_external void Platform_Draw();
ze_external void *Platform_Alloc(size_t size);
ze_external void Platform_Free(void* ptr);

ze_external ZEBuffer Platform_StageFile(char* path);

ze_external zErrorCode ZE_Init();

// config
ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc);

ze_external i32 ZCFG_Init(const char *cmdLine, const char **argv, const i32 argc);
ze_external i32 ZCFG_FindParamIndex(const char* shortQuery, const char* longQuery, i32 extraTokens);

// asset db
ze_external zErrorCode ZAssets_Init();
ze_external ZRTexture *ZAssets_AllocTex(i32 width, i32 height);
ze_external ZRMeshData* ZAssets_AllocMesh(i32 maxVerts);

ze_external void ZGen_Init();
ze_external void ZGen_FillTexture(ColourU32 *pixels, i32 w, i32 h, ColourU32 colour);
ze_external void ZGen_SetPixel(
    ColourU32 *pixels, i32 w, i32 h, ColourU32 colour, i32 x, i32 y);
ze_external void ZGen_FillTextureRect(
    ColourU32 *pixels, i32 w, i32 h, ColourU32 colour, Point2 topLeft, Point2 size);

ze_external i32 ZE_StartLoop();
ze_external void ZE_Shutdown();

ze_external zErrorCode ZR_Init();
ze_external zErrorCode ZR_Draw();

#endif // ZENGINE_INTERNAL_H

/*
Zealous Engine internal header
*/
#ifndef ZENGINE_INTERNAL_H
#define ZENGINE_INTERNAL_H

// Override the ze_common error handler
#if 1
#ifndef ZE_ASSERT
#define ZE_ASSERT(expression, msg)                                          \
    if (!(expression))                                                      \
    {                                                                       \
        char assertBuf[2048];                                               \
        snprintf(assertBuf, 2048, "%s, %d: %s\n", __FILE__, __LINE__, msg); \
        Platform_Fatal(assertBuf);                                          \
    }
#endif
#endif

// declared here so assert override can paste into headers
// messy, want to use ze_external here but its in the headers
// we are patching with the call to it...
// TODO: look into a better error handling scheme
extern "C" void Platform_Fatal(const char *msg);

// public engine core
#include "../../headers/zengine.h"

// scenes above this are internal - eg for debugging overlays
#define ZR_INTERNAL_SCENE_START_DEPTH 0x0FFFFFFF

#define ZR_MAX_BATCH_SIZE 256

//#define ZE_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__##)
#ifndef ZE_PRINTF
#define ZE_PRINTF(fmt, ...)
#endif

// internal types

//////////////////////////////////
// system events
//////////////////////////////////
// #define (sysEvPtr) ((Buffer))

#define ZE_SYS_EVENT_TYPE_NONE 0
#define ZE_SYS_EVENT_TYPE_INPUT 1

struct SysInputEvent
{
    BufferBlock header;
    u32 inputID = 0;
    i32 value = 0;
    f32 normalised = 0;
	frameInt frame;
};

////////////////////////////////////////////////////
// Concrete event types
////////////////////////////////////////////////////

static void Sys_EnqueueEvent(ZEBuffer *buf, BufferBlock *ev)
{
    ErrorCode err = BufBlock_Validate(ev);
    ZE_ASSERT(err == ZE_ERROR_NONE, "Invalid SysEvent")
    ZE_ASSERT(buf->Space() >= ev->size, "No space for SysEvent")
    //printf("SYS Enqueue ev %d size %d\n", ev->type, ev->size);
    buf->cursor += ZE_COPY(ev, buf->cursor, ev->size);
}

/*static void Sys_CreateInputEvent(SysInputEvent *ev, u32 inputID, i32 value, f32 normalised)
{
    BufBlock_PrepareHeader(&ev->header, sizeof(SysInputEvent), ZE_SYS_EVENT_TYPE_INPUT);
    ev->inputID = inputID;
    ev->value = value;
    ev->normalised = normalised;
}*/

static void Sys_WriteInputEvent(
	ZEBuffer *b,
	u32 inputID,
	i32 value,
	f32 normalised,
	frameInt frameNumber)
{
    SysInputEvent ev = {};
    // Sys_CreateInputEvent(&ev, inputID, value, normalised);
	BufBlock_PrepareHeader(&ev.header, sizeof(SysInputEvent), ZE_SYS_EVENT_TYPE_INPUT);
    ev.inputID = inputID;
    ev.value = value;
    ev.normalised = normalised;
	ev.frame = frameNumber;
    Sys_EnqueueEvent(b, BUF_BLOCK_CAST(&ev));
}

/*
struct SysPacketEvent
{
    BufferBlock header;
    i32 socketIndex;
    ZNetAddress sender;
    //number of bytes of data immediately following this struct
    i32 numBytes;
};

static void Sys_PrepareEvent(SysEvent *ev, i32 type, i32 size)
{
    ev->sentinel = SYS_EVENT_SENTINEL;
    ev->type = type;
    ev->size = size;
}
*/

//////////////////////////////////
// platform
ze_external void Platform_PollEvents();
ze_external void Platform_SubmitFrame();
ze_external void *Platform_Alloc(zeSize size);
ze_external void *Platform_Realloc(void* ptr, zeSize size);
ze_external void Platform_Free(void* ptr);
ze_external void Platform_DebugBreak();
ze_external ZEBuffer Platform_StageFile(const char *path);
ze_external f64 Platform_QueryClock();
ze_external void Platform_Sleep(i32 milliSeconds);


//////////////////////////////////
// window
ze_external zErrorCode ZWindow_Init();
ze_external void Window_SetCursorLock(i32 bLocked);
ze_external void Window_Shutdown();
ze_external ZScreenInfo Window_GetInfo();

//////////////////////////////////
// game module linkup
ze_external zErrorCode ZGame_Linkup(
    ZEngine engineImport, ZGame *gameExport, ZGameDef *gameDef);
ze_external zErrorCode ZGame_StubLinkup(
    ZEngine engineImport, ZGame *gameExport, ZGameDef *gameDef);
ze_external zErrorCode ZEngine_Init(
    ZSystem systemFunctions,
    ZFileIO ioFunctions,
    ZGame_LinkupFunction gameLink);

//////////////////////////////////
// debug
ze_external zErrorCode ZDebug_Init_1();
ze_external zErrorCode ZDebug_Init_2();

//////////////////////////////////
// config
ze_external zErrorCode ZE_InitConfig(const char *cmdLine, const char **argv, const i32 argc);

ze_external i32 ZCFG_Init(const char *cmdLine, const char **argv, const i32 argc);
ze_external ZConfig ZCFG_RegisterFunctions();
ze_external i32 ZCFG_RegisterTextCommands();

ze_external i32 ZCFG_FindParamIndex(const char* shortQuery, const char* longQuery, i32 extraTokens);
ze_external char *ZCFG_GetParamByIndex(const i32 index);
ze_external i32 ZCFG_FindIntParam(const char *shortQuery, const char *longQuery, i32 failResponse);

//////////////////////////////////
// console
ze_external ZTextCommand ZCmdConsole_RegisterFunctions();
ze_external i32 ZCmdConsole_Init();
ze_external i32 ZCmdConsole_Init_b();
ze_external zErrorCode ZCmdConsole_QueueCommand(char *cmd);
ze_external void ZCmdConsole_Execute();
ze_external zErrorCode ZCmdConsole_RegisterInternalCommand(
    char *name, char *description, ZCommand_Callback functionPtr);

ze_external void ZCmdConsole_SetInputEnabled(i32 flag);
ze_external i32 ZCmdConsole_GetInputEnabled();
ze_external void ZCmdConsole_WriteChar(char c, i32 bShiftOn);
ze_external void ZCmdConsole_SubmitText();

//////////////////////////////////
// input
ze_external ZInput ZInput_RegisterFunctions();
ze_external zErrorCode ZInput_Init();
ze_external void ZInput_ReadEvent(SysInputEvent* ev);

//////////////////////////////////
// asset db
ze_external ZRTexture *ZAssets_GetTexByName(char *name);
ze_external ZRTexture *ZAssets_GetTexById(i32 id);

ze_external ZRMeshAsset *ZAssets_GetMeshByName(char *name);
ze_external ZRMeshAsset *ZAssets_GetMeshById(i32 id);

ze_external ZRMaterial* ZAssets_GetMaterialByName(char* name);
ze_external ZRMaterial *ZAssets_GetMaterialById(i32 id);

ze_external ZRBlobAsset* ZAssets_GetBlobByName(char* name);
ze_external ZRBlobAsset* ZAssets_GetBlobById(i32 id);

ze_external ZRTexture* ZAssets_LoadPngTextureFromFile(const char* path);
ze_external void ZAssets_SaveImage(
    const char *fileName, i32 width, i32 height, const void *rgbPixels);

// asset allocation
ze_external ZRTexture *ZAssets_AllocTex(i32 width, i32 height, char *name);
ze_external ZRMeshAsset *ZAssets_AllocEmptyMesh(char *name, i32 maxVerts);
ze_external ZRBlobAsset* ZAssets_AllocBlob(char* name, zeSize numBytesA, zeSize numBytesB);
ze_external ZRMaterial *ZAssets_BuildMaterial(
    char *name, char *diffuseName, char *emissionName);

ze_external zErrorCode ZAssets_Init();
ze_external ZAssetManager ZAssets_RegisterFunctions();
ze_external void ZAssets_PrintAll();

ze_external zErrorCode ZEmbedded_Init();

//////////////////////////////////
// scene manager
ze_external ZSceneManager ZScene_RegisterFunctions();
ze_external void ZScene_Draw(ZRenderer renderer, ZGame game);

ze_external zeHandle ZScene_CreateScene(i32 order, i32 capacity, zeSize userBlobItemSize);
ze_external ZRDrawObj *ZScene_AddObject(zeHandle sceneHandle);
ze_external Transform ZScene_GetCamera(zeHandle sceneHandle);
ze_external void ZScene_SetCamera(zeHandle sceneHandle, Transform t);
ze_external void ZScene_SetProjection(zeHandle sceneHandle, M4x4 projection);
ze_external void ZScene_SetFlags(zeHandle handle, u32 flags);
ze_external u32 ZScene_GetFlags(zeHandle handle);
ze_external void ZScene_PostFrameTick();


ze_external i32 ZE_StartLoop();
ze_external void ZEngine_BeginShutdown();

// renderer
ze_external zErrorCode  ZR_Init();
ze_external void        ZR_ClearFrame(ColourF32 colour);
ze_external void        ZR_ExecuteCommands(ZEBuffer* commandBuffer);
ze_external void        ZR_Screenshot(char *fileName);
ze_external zErrorCode  ZR_DrawTest();
ze_external void        ZRGL_PrintHandles();
ze_external i32         ZR_GetGraphicsTestMode();

#endif // ZENGINE_INTERNAL_H

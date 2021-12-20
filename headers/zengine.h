/*
Zealous Engine public header
*/
#ifndef ZENGINE_H
#define ZENGINE_H

#include "ze_common.h"
#include "engine/zengine_types.h"
#include "engine/zengine_type_utils.h"
#include "engine/zengine_asset_gen.h"

///////////////////////////////////////////////////////////
// Embedded assets
///////////////////////////////////////////////////////////
// unit cube, -0.5 to 0.5 on x/y/z
#define ZE_EMBEDDED_CUBE_NAME "cube"
// unit quad, -0.5 to 0.5 on x/y
#define ZE_EMBEDDED_QUAD_NAME "quad"
// quad -1 to 1 on x/y to cover screenspace.
#define ZE_EMBEDDED_SCREEN_QUAD_NAME "screen_quad"

#define FALLBACK_MATERIAL_NAME "debug_magenta"
#define FALLBACK_CHEQUER_MATERIAL "debug_chequer"

#define FALLBACK_TEXTURE_NAME "fallback_texture"
#define FALLBACK_CHARSET_TEXTURE_NAME "fallback_charset"
#define FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME "fallback_charset_semi_transparent"

// 32 bit colours
#define COLOUR_U32_EMPTY { 0, 0, 0, 0 }
#define COLOUR_U32_WHITE { 255, 255, 255, 255 }
#define COLOUR_U32_BLACK { 0, 0, 0, 255 }
#define COLOUR_U32_RED { 255, 0, 0, 255 }
#define COLOUR_U32_GREEN { 0, 255, 0, 255 }
#define COLOUR_U32_BLUE { 0, 0, 255, 255 }
#define COLOUR_U32_YELLOW { 255, 255, 0, 255 }
#define COLOUR_U32_CYAN { 0, 255, 255, 255 }
#define COLOUR_U32_PURPLE { 255, 0, 255, 255 }
#define COLOUR_U32_GREY_LIGHT { 200, 200, 200, 255 }
#define COLOUR_U32_GREY { 100, 100, 100, 255 }
#define COLOUR_U32_GREY_DARK { 50, 50, 50, 255 }

#define COLOUR_U32_SEMI_GREY { 155, 155, 155, 155 }
#define COLOUR_U32_SEMI_GREY_DARK { 200, 200, 200, 200 }

// colors as Vec4s
#define COLOUR_F32_EMPTY { 0, 0, 0, 0 }
#define COLOUR_F32_WHITE { 1.f, 1.f, 1.f, 1.f }
#define COLOUR_F32_BLACK { 0, 0, 0, 1.f }
#define COLOUR_F32_RED { 1.f, 0, 0, 1.f }
#define COLOUR_F32_GREEN { 0, 1.f, 0, 1.f }
#define COLOUR_F32_BLUE { 0, 0, 1.f, 1.f }
#define COLOUR_F32_YELLOW { 1.f, 1.f, 0, 1.f }
#define COLOUR_F32_CYAN { 0, 1.f, 1.f, 1.f }
#define COLOUR_F32_PURPLE { 1.f, 0, 1.f, 1.f }

#define ZR_TEX_SAMPLER_DEFAULT 0

struct ZGameDef
{
    char* windowTitle;
    i32 targetFramerate;
	i32 bOverrideEscapeKey;
};

///////////////////////////////////////////
// game functions provided to engine
///////////////////////////////////////////
struct ZGame
{
    void (*Init)();
    void (*Shutdown)();
    void (*Tick)(ZEFrameTimeInfo timing);
    i32 sentinel;
};


///////////////////////////////////////////
// Engine objects provided to game DLL
///////////////////////////////////////////

typedef void (*ZCommand_Callback)(char* fullString, char** tokens, i32 numTokens);

#define ZCMD_CALLBACK(functionName) \
internal void functionName(char* fullString, char** tokens, i32 numTokens)

struct ZConfig
{
    i32 (*FindParamIndex)(const char *shortQuery, const char *longQuery, i32 extraTokens);
    char* (*GetParamByIndex)(const i32 index);
    i32 (*FindIntParam)(const char *shortQuery, const char *longQuery, i32 failResponse);
};

struct ZTextCommand
{
    zErrorCode (*RegisterCommand)(
        char *name, char *description, ZCommand_Callback functionPtr);
    zErrorCode (*QueueCommand)(char *cmd);
};

struct ZFileIO
{
	// if handle == 0, open failed
	zeHandle	(*Open)(char* path);
	void		(*Close)(zeHandle fileHandle);
	u32			(*Measure)(zeHandle fileHandle);
	void		(*Seek)(zeHandle fileHandle, u32 position);
    i32         (*Read)(zeHandle fileHandle, i32 numBytes, ZEBuffer* target);
	
};

struct ZInput
{
    void (*AddAction)(u32 keyCode1, u32 keyCode2, char *label);
    i32 (*GetActionValue)(char *actionName);
    f32 (*GetActionValueNormalised)(char *actionName);

    i32 (*HasActionToggledOn)(char* actionName, frameInt frameNumber);
    // i32 (*HasActionToggledOff)(char* actionName);
	
	const char* (*GetInputLongLabel)(i32 code);
	const char* (*GetInputShortLabel)(i32 code);
};

// Services
struct ZSceneManager
{
    zeHandle (*AddScene)(i32 order, i32 capacity, zeSize userDataBytesPerObject);
    ZRDrawObj *(*AddObject)(zeHandle scene);
    ZRDrawObj *(*GetObject)(zeHandle scene, zeHandle objectId);
    void (*RemoveObject)(zeHandle scene, zeHandle objectId);

    i32 (*GetObjectCount)(zeHandle sceneHandle);
    ZRDrawObj *(*GetObjectByIndex)(zeHandle sceneHandle, i32 i);

    Transform (*GetCamera)(zeHandle sceneHandle);
    void (*SetCamera)(zeHandle sceneHandle, Transform t);
    void (*SetProjection)(zeHandle sceneHandle, M4x4 projection);
	void (*SetClearColour)(ColourF32 colour);

    // utilities
    ZRDrawObj* (*AddCube)(zeHandle scene, char* materialName);
    ZRDrawObj* (*AddFullTextureQuad)(zeHandle scene, char *textureName, Vec2 size);
    ZRDrawObj *(*AddLinesObj)(zeHandle scene, i32 maxVerts);
};

struct ZAssetManager
{
    ZRTexture *(*GetTexByName)(char *name);
    ZRTexture *(*GetTexById)(i32 id);

    ZRMeshAsset *(*GetMeshByName)(char *name);
    ZRMeshAsset *(*GetMeshById)(i32 id);

    ZRMaterial *(*GetMaterialById)(i32 id);
    ZRMaterial *(*GetMaterialByName)(char *name);

    ZRBlobAsset* (*GetBlobByName)(char* name);
    ZRBlobAsset* (*GetBlobById)(i32 id);

    ZRTexture *(*AllocTexture)(i32 width, i32 height, char *name);
    ZRMaterial *(*AllocMaterial)(char* name);
    ZRMeshAsset *(*AllocEmptyMesh)(char *name, i32 maxVerts);
    ZRBlobAsset* (*AllocBlob)(char* name, zeSize numBytesA, zeSize numBytesB);

    ZRMaterial* (*BuildMaterial)(
        char* name, char* diffuseName, char* emissionName);
};

struct ZSystem
{
    void* (*Malloc)(zeSize size);
    void* (*Realloc)(void* ptr, zeSize size);
    void (*Free)(void* ptr);
    f64 (*QueryClock)();
};

// engine functions provided to game
struct ZEngine
{
    ZSceneManager scenes;
    ZAssetManager assets;
    ZConfig cfg;
    ZTextCommand textCommands;
    ZInput input;
    ZSystem system;
    ZFileIO io;
    i32 sentinel;
};

#define ZGAME_DLL_NAME "game.dll"
#define ZGAME_BASE_DIRECTORY "base"
#define ZGAME_LINKUP_FUNCTION_NAME "ZGameLinkUp"

#define ZSCENE_FLAG_NO_DRAW (1 << 0)

#define Z_GAME_WINDOWS_LINK_FUNCTION \
extern "C" zErrorCode __declspec(dllexport) ZGameLinkUp(ZEngine engineImport, ZGame *gameExport, ZGameDef *gameDef)

// Signature of linking function game DLL must export
typedef zErrorCode(ZGame_LinkupFunction)(ZEngine engineImport, ZGame *gameExport, ZGameDef *gameDef);

typedef zErrorCode(ZE_EventCallback)(i32 code, void *data, zeSize dataSize);

#endif // ZENGINE_H

/*
Zealous Engine public header
*/
#ifndef ZENGINE_H
#define ZENGINE_H

#include "ze_common.h"
#include "engine/zengine_types.h"
#include "engine/zengine_asset_gen.h"

///////////////////////////////////////////////////////////
// Embedded assets
///////////////////////////////////////////////////////////
#define ZE_EMBEDDED_CUBE_NAME "cube"

#define FALLBACK_MATERIAL_NAME "debug_magenta"
#define FALLBACK_CHEQUER_MATERIAL "debug_chequer"

#define FALLBACK_TEXTURE_NAME "fallback_texture"
#define FALLBACK_CHARSET_TEXTURE_NAME "fallback_charset"
#define FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME "fallback_charset_semi_transparent"


#define COLOUR_F32_EMPTY { 0, 0, 0, 0 }
#define COLOUR_F32_WHITE { 1.f, 1.f, 1.f, 1.f }
#define COLOUR_F32_BLACK { 0, 0, 0, 1.f }
#define COLOUR_F32_RED { 1.f, 0, 0, 1.f }
#define COLOUR_F32_GREEN { 0, 1.f, 0, 1.f }
#define COLOUR_F32_BLUE { 0, 0, 1.f, 1.f }
#define COLOUR_F32_YELLOW { 1.f, 1.f, 0, 1.f }
#define COLOUR_F32_CYAN { 0, 1.f, 1.f, 1.f }
#define COLOUR_F32_PURPLE { 1.f, 0, 1.f, 1.f }

#define COLOUR_U32_EMPTY { 0, 0, 0, 0 }
#define COLOUR_U32_WHITE { 255, 255, 255, 255 }
#define COLOUR_U32_BLACK { 0, 0, 0, 255 }
#define COLOUR_U32_RED { 255, 0, 0, 255 }
#define COLOUR_U32_GREEN { 0, 255, 0, 255 }
#define COLOUR_U32_BLUE { 0, 0, 255, 255 }
#define COLOUR_U32_YELLOW { 255, 255, 0, 255 }
#define COLOUR_U32_CYAN { 0, 255, 255, 255 }
#define COLOUR_U32_PURPLE { 255, 0, 255, 255 }
#define COLOUR_U32_GREY { 155, 155, 155, 255 }
#define COLOUR_U32_GREY_DARK { 200, 200, 200, 255 }

#define COLOUR_U32_SEMI_GREY { 155, 155, 155, 155 }
#define COLOUR_U32_SEMI_GREY_DARK { 200, 200, 200, 200 }

#define ZR_TEX_SAMPLER_DEFAULT 0

struct ZGameDef
{
    char* windowTitle;
    i32 targetFramerate;
};

// game functions provided to engine
struct ZGame
{
    void (*Init)();
    void (*Shutdown)();
    void (*Tick)(ZEFrameTimeInfo timing);
    i32 sentinel;
};

struct ZInput
{
    void (*AddAction)(u32 keyCode1, u32 keyCode2, char *label);
    i32 (*GetActionValue)(char *actionName);
    f32 (*GetActionValueNormalised)(char *actionName);
    i32 (*HasActionToggledOn)(char* actionName);
    i32 (*HasActionToggledOff)(char* actionName);
};

// Services
struct ZSceneManager
{
    zeHandle (*AddScene)(i32 order, i32 capacity);
    ZRDrawObj *(*AddObject)(zeHandle scene);
    ZRDrawObj *(*GetObject)(zeHandle scene, zeHandle objectId);
    Transform (*GetCamera)(zeHandle sceneHandle);
    void (*SetCamera)(zeHandle sceneHandle, Transform t);
    void (*SetProjection)(zeHandle sceneHandle, M4x4 projection);
};

struct ZAssetManager
{
    ZRTexture *(*GetTexByName)(char *name);
    ZRTexture *(*GetTexById)(i32 id);

    ZRMeshAsset *(*GetMeshByName)(char *name);
    ZRMeshAsset *(*GetMeshById)(i32 id);

    ZRMaterial *(*GetMaterialById)(i32 id);
    ZRMaterial *(*GetMaterialByName)(char *name);

    ZRTexture *(*AllocTexture)(i32 width, i32 height, char *name);
    ZRMaterial *(*AllocMaterial)(char* name);
    ZRMeshAsset *(*AllocEmptyMesh)(char *name, i32 maxVerts);
    ZRMaterial* (*BuildMaterial)(
        char* name, char* diffuseName, char* emissionName);
};

struct ZAssetGen
{
    void (*ZGen_DrawLine)(ZRTexture *tex, ColourU32 colour, Point2 a, Point2 b);
    void (*ZGen_FillTextureRect)(ZRTexture *tex, ColourU32 colour, Point2 topLeft, Point2 size);
    void (*ZGen_FillTexture)(ZRTexture *tex, ColourU32 colour);
};

// engine functions provided to game
struct ZEngine
{
    ZSceneManager scenes;
    ZAssetManager assets;
    ZInput input;
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

#endif // ZENGINE_H

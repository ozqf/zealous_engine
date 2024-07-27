/*
Zealous Engine public header
*/
#ifndef ZENGINE_H
#define ZENGINE_H

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
#define FALLBACK_TEXTURE_WHITE "fallback_texture_white"
#define FALLBACK_CHARSET_TEXTURE_NAME "fallback_charset"
#define FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME "fallback_charset_semi_transparent"

#define DEFAULT_SHADER_FLAG_ALBEDO_COLOUR_ONLY (1 << 0)

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
#define COLOUR_F32_EMPTY { 0.f, 0.f, 0.f, 0.f }
#define COLOUR_F32_WHITE { 1.f, 1.f, 1.f, 1.f }
#define COLOUR_F32_BLACK { 0.f, 0.f, 0.f, 1.f }
#define COLOUR_F32_RED { 1.f, 0.f, 0.f, 1.f }
#define COLOUR_F32_GREEN { 0.f, 1.f, 0.f, 1.f }
#define COLOUR_F32_BLUE { 0.f, 0.f, 1.f, 1.f }
#define COLOUR_F32_YELLOW { 1.f, 1.f, 0.f, 1.f }
#define COLOUR_F32_CYAN { 0.f, 1.f, 1.f, 1.f }
#define COLOUR_F32_PURPLE { 1.f, 0.f, 1.f, 1.f }

#define COLOUR_F32_ORANGE { 1.f, 0.5f, 0.f, 1.f }
#define COLOUR_F32_LIGHT_GREY { 0.5f, 0.5f, 0.5f, 1.f }

#define ZR_TEX_SAMPLER_DEFAULT 0
#define ZR_DEFAULT_FOV 100.f

#include "ze_common.h"
#include "engine/zengine_types.h"
#include "engine/zengine_type_utils.h"
#include "engine/zengine_asset_gen.h"

/////////////////////////////////////////
// Render commands
/////////////////////////////////////////
#define ZR_DRAW_CMD_NONE 0
#define ZR_DRAW_CMD_SET_CAMERA 1
#define ZR_DRAW_CMD_SPRITE_BATCH 2
#define ZR_DRAW_CMD_MESH 3
#define ZR_DRAW_CMD_DEBUG_LINES 4
#define ZR_DRAW_CMD_CLEAR_BUFFER 5

struct ZRDrawCmdSetCamera
{
    BufferBlock header;
    Transform camera;
    M4x4 projection;
};

struct ZRDrawCmdMesh
{
    BufferBlock header;
    ZRDrawObj obj;
};

struct ZRDrawCmdDebugLines
{
	BufferBlock header;
	// if lines are chained, lines will be drawn between verts
	// otherwise, lines will be drawn independently between vertex pairs
	// chained: a->b->c->d->e
	// unchained: a->b c->d (e has no pair and is ignored)
	i32 bChained;
	i32 numVerts;
	ZRLineVertex* verts;
};

struct ZRDrawCmdClearBuffer
{
    BufferBlock header;
};

struct ZRSpriteBatchItem
{
    Vec3 pos;
    Vec2 size;
    Vec2 uvMin;
    Vec2 uvMax;
    f32 radians;
    ColourF32 colour;
};

struct ZRDrawCmdSpriteBatch
{
    BufferBlock header;
    i32 textureId;
    i32 numItems;
    ZRSpriteBatchItem* items;

    void AddItem(Vec3 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax, f32 radians, ColourF32 colour)
    {
        items[numItems].pos = pos;
        items[numItems].size = size;
        items[numItems].uvMin = uvMin;
        items[numItems].uvMax = uvMax;
        items[numItems].radians = radians;
        items[numItems].colour = colour;
        numItems++;
    }

    zeSize Finish(ZEBuffer* buf)
    {
        this->header.size = sizeof(ZRDrawCmdSpriteBatch) + (sizeof(ZRSpriteBatchItem) * numItems);
        buf->cursor = (i8*)this + this->header.size;
        return this->header.size;
    }
};

struct ZRenderer
{
	void (*ExecuteCommands)(ZEBuffer* commandBuffer);	
};

#define GAME_DEF_FLAG_OVERRIDE_ESCAPE_KEY (1 << 0)
#define GAME_DEF_FLAG_MANUAL_RENDER (1 << 1)

struct ZGameDef
{
    char* windowTitle;
    u32 flags;
    i32 targetFramerate;
};

///////////////////////////////////////////
// game functions provided to engine
///////////////////////////////////////////
struct ZGame
{
    void (*Init)();
    void (*Shutdown)();
    void (*Tick)(ZEFrameTimeInfo timing);
    void (*Draw)(ZRenderer renderer);
    i32 sentinel;
};


///////////////////////////////////////////
// Engine objects provided to game DLL
///////////////////////////////////////////

typedef void (*ZCommand_Callback)(char* fullString, char** tokens, i32 numTokens);

#define ZCMD_CALLBACK(functionName) \
internal void functionName(char* fullString, char** tokens, i32 numTokens)

/*
Stores commanad line parameters.
In future will store saved key/value pairs for configuration
*/
struct ZConfig
{
    i32 (*FindParamIndex)(const char *shortQuery, const char *longQuery, i32 extraTokens);
    char* (*GetParamByIndex)(const i32 index);
    i32 (*FindIntParam)(const char *shortQuery, const char *longQuery, i32 failResponse);
};

/*
Text commands are queued and executed at the start of every frame.
*/
struct ZTextCommand
{
    zErrorCode (*RegisterCommand)(
        char *name, char *description, ZCommand_Callback functionPtr);
    zErrorCode (*QueueCommand)(char *cmd);
};

struct ZFileIO
{
    // TODO: Just slamming any file read directly into RAM at the moment

    // reading
	// if handle == 0, open failed
	//zeHandle	(*Open)(char* path);
	//void		(*Close)(zeHandle fileHandle);
	//u32			(*Measure)(zeHandle fileHandle);
	//void		(*Seek)(zeHandle fileHandle, u32 position);
    //i32         (*Read)(zeHandle fileHandle, i32 numBytes, ZEBuffer* target);
    ZEBuffer (*StageFile)(const char* path);

    // writing
    zErrorCode  (*WriteFile)(const char* path, void* data, zeSize numBytes);
};

struct ZInput
{
    void (*AddAction)(u32 keyCode1, u32 keyCode2, char *label);
    i32 (*GetActionValue)(char *actionName);
    f32 (*GetActionValueNormalised)(char *actionName);

    // TODO: Remove janky need to pass in the current framenumber!
    i32 (*HasActionToggledOn)(char* actionName, frameInt frameNumber);
    // i32 (*HasActionToggledOff)(char* actionName);
	
	const char* (*GetInputLongLabel)(i32 code);
	const char* (*GetInputShortLabel)(i32 code);
	
	void (*SetCursorLocked)(i32 bCursorLocked);
};

struct ZSceneManager
{
    // add/remove scenes and objects within them
    // removing scenes not supported yet... not needed it
    zeHandle (*AddScene)(i32 order, i32 capacity, zeSize userDataBytesPerObject);
    ZRDrawObj *(*AddObject)(zeHandle scene);
    ZRDrawObj *(*GetObject)(zeHandle scene, zeHandle objectId);
    void (*RemoveObject)(zeHandle scene, zeHandle objectId);

    // for iteration
    i32 (*GetObjectCount)(zeHandle sceneHandle);
    ZRDrawObj *(*GetObjectByIndex)(zeHandle sceneHandle, i32 i);

    // scene properties
    Transform (*GetCamera)(zeHandle sceneHandle);
    void (*SetCamera)(zeHandle sceneHandle, Transform t);
    void (*SetProjectionManual)(zeHandle sceneHandle, M4x4 projection);
    void (*SetProjection3D)(zeHandle sceneHandle, f32 fov);
    void (*SetProjectionOrtho)(zeHandle sceneHandle, f32 verticalExtent);
	void (*SetClearColour)(ColourF32 colour);
    u32 (*GetSceneFlags)(zeHandle sceneHandle);
    void (*SetSceneFlags)(zeHandle sceneHandle, u32 flags);
    void (*SetSceneFlag)(zeHandle sceneHandle, i32 mask, i32 bValue);
    void (*ApplyDefaultOrthoProjection)(zeHandle handle, f32 verticalExtent, f32 aspectRatio);

    // utilities for creating basic draw objects
    ZRDrawObj* (*AddCube)(zeHandle scene, char* materialName);
    ZRDrawObj* (*AddFullTextureQuad)(zeHandle scene, char *textureName, Vec2 size, ColourF32 colour);
    ZRDrawObj *(*AddLinesObj)(zeHandle scene, i32 maxVerts);
};

/*
TODO: Assets current do not store their original name, only the has of the name
it uses to look the asset up.
*/
struct ZAssetManager
{
    ZRTexture *(*GetTexByName)(char *name);
    ZRTexture *(*GetTexById)(i32 id);

    ZRMeshAsset *(*GetMeshByName)(char *name);
    ZRMeshAsset *(*GetMeshById)(i32 id);

    ZRMaterial *(*GetMaterialById)(i32 id);
    ZRMaterial *(*GetMaterialByName)(char *name);

    // TODO: ZRBlobAsset - not actually used now?
    ZRBlobAsset* (*GetBlobByName)(char* name);
    ZRBlobAsset* (*GetBlobById)(i32 id);

    ZRTexture *(*AllocTexture)(i32 width, i32 height, char *name);
    ZRMaterial *(*AllocMaterial)(char* name);
    ZRMeshAsset *(*AllocEmptyMesh)(char *name, i32 maxVerts);
    ZRBlobAsset* (*AllocBlob)(char* name, zeSize numBytesA, zeSize numBytesB);

    ZRMaterial* (*BuildMaterial)(
        char* name, char* diffuseName, char* emissionName);
    
    zErrorCode (*LoadTextureFromFile)(const char* path, ZRTexture** result);
};

struct ZSystem
{
    void* (*Malloc)(zeSize size);
    void* (*Realloc)(void* ptr, zeSize size);
    void (*Free)(void* ptr);
    f64 (*QueryClock)();
    ZScreenInfo (*GetScreenInfo)();

    // When something unrecoverable has happened
    void (*Fatal)(const char *msg);
    // Crash functions are called after a call to 'Fatal' to allow
    // writing out debug info.
    void (*RegisterCrashDumpFunction)(ZE_CrashDumpFunction fn);
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

// if no game directory parameter is provided, look in this dir for a game dll
#define ZGAME_BASE_DIRECTORY "base"
#define ZGAME_DLL_NAME "game.dll"
#define ZGAME_LINKUP_FUNCTION_NAME "ZGameLinkUp"

#define ZSCENE_FLAG_NO_DRAW (1 << 0)

#define Z_GAME_WINDOWS_LINK_FUNCTION \
extern "C" zErrorCode __declspec(dllexport) ZGameLinkUp(ZEngine engineImport, ZGame *gameExport, ZGameDef *gameDef)

// Signature of linking function game DLL must export
typedef zErrorCode(ZGame_LinkupFunction)(ZEngine engineImport, ZGame *gameExport, ZGameDef *gameDef);

typedef zErrorCode(ZE_EventCallback)(i32 code, void *data, zeSize dataSize);

extern "C" int ZE_GetVersion();
extern "C" int ZE_Startup(char* lpCmdLine);



#endif // ZENGINE_H

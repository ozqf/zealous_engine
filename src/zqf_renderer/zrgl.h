#ifndef ZRGL_H
#define ZRGL_H

#include "../zqf_renderer.h"

/////////////////////////////////////////////////////////////
// Renderer module shared data types
/////////////////////////////////////////////////////////////

// Hold memory and handle to a texture
struct ZRDataTexture
{
    u32 handle;
    i32 bIs1D;
    i32 width;
    i32 height;
    Vec4* mem;
    //Point2 cursor;
    i32 cursor;
    i32 numBytes;
};

#if 0
#pragma pack(push, 1)
// each Vec4 is a pixel in the data texture
struct ZRDrawObjBatchLightInfo
{
    Vec4 viewPos;
    Vec4 colour;
    Vec4 settings;
};

// data for each object in a batch, uploaded to data texture
// All data types must be Vec4s!
struct ZRDrawObjBatchParams
{
    Vec4 modelView[4];
    Vec4 ambientColour;
    ZRDrawObjBatchLightInfo lights[ZR_MAX_POINT_LIGHTS_PER_MODEL];
};

struct ZRShaderParams_LitMesh
{
	f32 modelView[16];
    Vec4 ambientColour;
    ZRDrawObjBatchLightInfo lights[ZR_MAX_POINT_LIGHTS_PER_MODEL];
};
#pragma pack(pop)
#endif

///////////////////////////////////////////////////////////
// Profiling data
///////////////////////////////////////////////////////////
struct ZRGroupingStats
{
    i32 numGroups;
    i32 numObjects;
    i32 numLights;
    i32 lightsWritten;
    f64 time;
    i32 shadowMaps;
    i32 drawCallsShadows;
    i32 drawCallsGBuffer;
    f64 gBufferFillMS;
    f64 gBufferLightMS;
};

struct ZRPerformanceStats
{
    u32 trisSingle;
    u32 trisBatched;
    u32 trisTotal;

    f64 prepareTime;
    f64 uploadTime;
    f64 drawTime;
    i32 drawCalls;
    f32 dataTexPercentUsed;
    f64 total;
	i32 numScenes;
	i32 numGroups;

    i32 listBytes;
    i32 numListBytes;

    ZRGroupingStats grouping;
};

///////////////////////////////////////////////////////////
// Imported platform functions
// - passed in at initialisation
///////////////////////////////////////////////////////////
struct ZRPlatform
{
    void (*Error)(const char* msg);
    double (*QueryClock)();
    // void* (*Allocate)(i32 numBytes);
    // void (*Free)(void* ptr);
    ZRAssetDB* (*GetAssetDB)();
    void (*DebugBreak)();
};

///////////////////////////////////////////////////////////
// Draw Commands - written by backend, executed by frontend
///////////////////////////////////////////////////////////

/**
 * Contents of string is written behind the command.
 * String must be null terminated
 * command bytes == size of cmd struct + string itself
 */
struct ZRDrawCmd_Text
{
    Vec3 origin;
    i32 numChars; // includes null terminator
    f32 charSize;
    f32 aspectRatio;
    i32 offsetToString;
    i32 alignmentMode;
    i32 charsetTextureId;
    Colour fontColour;
    Colour bgColour;
};


/////////////////////////////////////////////////////////////
// Some shared utility functions
/////////////////////////////////////////////////////////////

inline void ZR_BuildModelMatrix(M4x4* model, Transform* modelT)
{
	// Model
	M4x4_SetToIdentity(model->cells);
	Vec3 modelEuler = M3x3_GetEulerAnglesRadians(modelT->rotation.cells);
	// model translation
	M4x4_Translate(model->cells, modelT->pos.x, modelT->pos.y, modelT->pos.z);
	// model rotation
	M4x4_RotateByAxis(model->cells, modelEuler.y, 0, 1, 0);
	M4x4_RotateByAxis(model->cells, modelEuler.x, 1, 0, 0);
	M4x4_RotateByAxis(model->cells, modelEuler.z, 0, 0, 1);
	M4x4_ApplyScale(model->cells, modelT->scale.x, modelT->scale.y, modelT->scale.z);
}

inline void ZR_BuildViewMatrix(M4x4* view, Transform* camT)
{
	// View
	M4x4_SetToIdentity(view->cells);
	Vec3 camEuler = M3x3_GetEulerAnglesRadians(camT->rotation.cells);
	M4x4_RotateByAxis(view->cells, -camEuler.z, 0, 0, 1);
	M4x4_RotateByAxis(view->cells, -camEuler.x, 1, 0, 0);
	M4x4_RotateByAxis(view->cells, -camEuler.y, 0, 1, 0);
	// inverse camera translation
	M4x4_Translate(view->cells, -camT->pos.x, -camT->pos.y, -camT->pos.z);
}

/////////////////////////////////////////////////////////////
// Renderer export
/////////////////////////////////////////////////////////////

extern "C" ErrorCode ZRGL_Init(i32 scrWidth, i32 scrHeight);
extern "C" void ZRGL_UpdateStats(f64 swapMS, f64 frameMS);
extern "C" void ZRGL_CheckForUploads(i32 bVerbose);
extern "C" i32 ZRGL_ExecTextCommand(
	const char* str, const char** tokens, const i32 numTokens);

// Add a scene to draw the current command console text
extern "C" void ZRGL_AppendConsoleScene(ZEBuffer* list, ZEBuffer* data, char* text);

extern "C" void ZRGL_Link(ZRPlatform platform);

extern "C" ZRPerformanceStats ZRGL_DrawFrame(
	ZEBuffer* drawList,
    ZEBuffer* drawData,
    ScreenInfo scrInfo);

#endif // ZRGL_H
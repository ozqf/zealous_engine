#ifndef _ZQF_RENDERER_H
#define _ZQF_RENDERER_H

/*
ZRenderer external interface

TODO: Rationalise naming of functions and data structures!

TODO: This renderer is using some data types and functions from the common module
These shouldn't be in common! Move them into this module
 */
#include "../headers/common/ze_common_full.h"

#define ZR_SENTINEL 0xDEADBEEF

#define ZR_TARGET_FRAME_TIME 16.67

//#define ZR_REPORT_GROUP_ERRORS

// TODO: Changing this breaks things atm
#define ZR_MAX_POINT_LIGHTS_PER_MODEL 8

// If lighting data is transferred via data texture, this is the number of pixels (Vec4s)
// per light.
#define ZR_DATA_PIXELS_PER_LIGHT 3

struct ScreenInfo
{
    i32 width;
    i32 height;
    f32 aspectRatio;
};

#define ZR_MAX_PREFABS 64

#define ZR_CUBEMAP_LOAD_INDEX_RIGHT 0
#define ZR_CUBEMAP_LOAD_INDEX_LEFT 1

#define ZR_CUBEMAP_LOAD_INDEX_TOP 2
#define ZR_CUBEMAP_LOAD_INDEX_BOTTOM 3

#define ZR_CUBEMAP_LOAD_INDEX_BACK 4
#define ZR_CUBEMAP_LOAD_INDEX_FRONT 5

#define ZR_PREFAB_FLAG_DYNAMIC_MESH (1 << 0)

#define ZR_PROJECTION_MODE_3D 0
#define ZR_PROJECTION_MODE_IDENTITY 1
#define ZR_PROJECTION_MODE_ORTHO_BASE 2

#define ZR_MAX_BATCH_SIZE 256
//#define ZR_MAX_BATCH_SIZE 100

#include "zr_draw_types.h"

///////////////////////////////////////////////////////////////////////////////////
// Draw Frame types:
///////////////////////////////////////////////////////////////////////////////////

/*
Buffers:
- Draw list -
ZRViewFrame - lists scene frames
	ZRSceneFrame - lists objects and static environment eg projection, camera etc
		ZRDrawObj
		ZRDrawObj
		ZRDrawObj
    ZRSceneFrame
		ZRDrawObj
		ZRDrawObj
		ZRDrawObj
In memory:
ZRViewFrame|ZRSceneFrame|Obj|Obj|Obj|ZRSceneFrame|Obj|Obj...

- Scratch - 
Stack of random allocations for objects in the scene lists.
Eg text objects will write their strings into here.
Contents is nonsense without the draw frame objects.
*/

struct ZRDrawGroup
{
	// The prefab these objects are grouped on
	//i32 prefab;
    //ZRGroupId id;
    u32 hash;
    ZRDrawObjData data;
	// location in buffer for this group's command + data
	u8* commandPtr;
    u8* shaderDataPtr;
    i32 bBatchable;
    //ZRShader* shader;
	// bytes reserved for this command should equal:
	// sizeof(Command) + (sizeof(data per instance) * numItems)
	i32 reservedBytes;

    i32 dataPixelIndex;
    i32 pixelsPerItem;
	
    i32 numItems;
	i32 indices[ZR_MAX_BATCH_SIZE];
};

#define ZR_MAX_DRAW_GROUPS 256

struct ZRSceneView
{
    i32 numGroups = 0;
    i32 numLights;
	ZRDrawGroup* groups[ZR_MAX_DRAW_GROUPS];
	i32 lights[ZR_MAX_DRAW_GROUPS];
};

struct ZRSceneFrame
{
    // must be set by caller to renderer
    u32 sentinel;
    struct
    {
        i32 projectionMode;
        i32 numObjects;
        ZRDrawObj* objects;
        // num data bytes is stored but since draw obj is a union, could just
        // do sizeof(ZRDrawObj) * numObjects
        i32 numListBytes;
        i32 bSkybox;
        i32 bDeferred;
        i32 bIsInteresting;
        f32 timestamp;
        Transform camera;
    } params;
    
    // reserved space in buffer used by renderer when preprocessing scene
    struct
    {
        ZRDrawObj* objects;
        ZRSceneView* view;
        M4x4 projection;
    } drawTime;
    
};

// Describes a set of scenes to draw, one after the other
// eg game/model/hud
struct ZRViewFrame
{
    u32 sentinel;   // should equal ZR_SENTINEL or is invalid
    f64 prebuildTime;
    i32 bVerbose;   // dump debug info whilst rendering this frame
    i32 numScenes;
    i32 frameNumber; // identifies a specific view frame from another
    timeFloat timestamp;
    // pointers to the two buffers, draw list and scratch data.
    ZEBuffer* list;
    ZEBuffer* data;
};

///////////////////////////////////////////////////////////
// Render commands and resources
///////////////////////////////////////////////////////////

static ZRSceneFrame* ZRScene_InitInPlace(
    ZEBuffer* list, i32 projectionMode, i32 bDeferred)
{
    ZRSceneFrame* scene = (ZRSceneFrame*)list->cursor;
    list->cursor += sizeof(ZRSceneFrame);
    *scene = {};
    scene->sentinel = ZR_SENTINEL;
    scene->params.bDeferred = bDeferred;
    scene->params.bIsInteresting = NO;
    scene->params.bSkybox = YES;
    scene->params.projectionMode = projectionMode;
    //Transform_SetToIdentity(&scene->params.camera);
    scene->params.objects = (ZRDrawObj*)list->cursor;
    return scene;
}

#include "zr_asset_db.h"

#endif // _ZQF_RENDERER_H
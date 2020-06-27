#ifndef _ZQF_RENDERER_H
#define _ZQF_RENDERER_H

/*
ZRenderer external interface

TODO: Rationalise naming of functions and data structures!

TODO: This renderer is using some data types and functions from the common module
These shouldn't be in common! Move them into this module
 */
#include "ze_common/ze_common.h"
#include "ze_common/ze_byte_buffer.h"
#include "ze_common/ze_math_types.h"
#include "ze_common/ze_memory_utils.h"

#define ZR_SENTINEL 0xDEADBEEF

#define ZR_TARGET_FRAME_TIME 16.67

//#define ZR_REPORT_GROUP_ERRORS

// TODO: Changing this breaks things atm
#define ZR_MAX_POINT_LIGHTS_PER_MODEL 8

// If lighting data is transferred via data texture, this is the number of pixels (Vec4s)
// per light.
#define ZR_DATA_PIXELS_PER_LIGHT 3

// struct ZRSceneView;
// struct ZRShader;
// struct ZRMaterial;
// struct ZRPlatform;

struct ScreenInfo
{
    i32 width;
    i32 height;
    f32 aspectRatio;
};

#define ZR_MAX_PREFABS 64

// Indices to render pre-configured objects
#define ZR_PREFAB_TYPE_CUBE 0
#define ZR_PREFAB_TYPE_INVERSE_CUBE 1
#define ZR_PREFAB_TYPE_WALL 2
#define ZR_PREFAB_TYPE_GUN 3
#define ZR_PREFAB_TYPE_ORIENTATION_TEST 4
#define ZR_PREFAB_TYPE_PILLAR 5
#define ZR_PREFAB_TYPE_BLOCK_COLOURED 6
#define ZR_PREFAB_TYPE_QUAD 7
#define ZR_PREFAB_TYPE_QUAD_DYNAMIC 8
#define ZR_PREFAB_TYPE_MAGE_TEST 9
#define ZR_PREFAB_TYPE_SPIKE 10
#define ZR_PREFAB_TYPE_DEBUG_PLAYER 11
#define ZR_PREFAB_TYPE_DEBUG_WALL 12
#define ZR_PREFAB_TYPE_DEBUG_ENEMY 13
#define ZR_PREFAB_TYPE_DEBUG_ENEMY_PROJECTILE 14
#define ZR_PREFAB_TYPE_DEBUG_PLAYER_PROJECTILE 15
#define ZR_PREFAB_TYPE_DEBUG_ITEM 16
#define ZR_PREFAB_TYPE_DEBUG_EXPLOSION 17
#define ZR_PREFAB_TYPE_DEBUG_BOUNDING_BOX 18
#define ZR_PREFAB_TYPE_SPHERE 19

#define ZR_CUBEMAP_LOAD_INDEX_RIGHT 0
#define ZR_CUBEMAP_LOAD_INDEX_LEFT 1

#define ZR_CUBEMAP_LOAD_INDEX_TOP 2
#define ZR_CUBEMAP_LOAD_INDEX_BOTTOM 3

#define ZR_CUBEMAP_LOAD_INDEX_BACK 4
#define ZR_CUBEMAP_LOAD_INDEX_FRONT 5

#define ZR_PREFAB_FLAG_DYNAMIC_MESH (1 << 0)


#define ZR_MAX_PROGRAMS 128

#define ZR_SHADER_TYPE_NONE -1
#define ZR_SHADER_TYPE_FALLBACK 0
#define ZR_SHADER_TYPE_TEST 1
#define ZR_SHADER_TYPE_BLOCK_COLOUR 2
#define ZR_SHADER_TYPE_BATCHED 3
#define ZR_SHADER_TYPE_TEXT 4
#define ZR_SHADER_TYPE_SKYBOX 5
#define ZR_SHADER_TYPE_BLOCK_COLOUR_BATCHED 6
#define ZR_SHADER_TYPE_SHADOW_MAP 7
#define ZR_SHADER_TYPE_SHADOW_MAP_DEBUG 8
#define ZR_SHADER_TYPE_BUILD_GBUFFER 9
#define ZR_SHADER_TYPE_COMBINE_GBUFFER 11
#define ZR_SHADER_TYPE_GBUFFER_LIGHT_DIRECT 12
#define ZR_SHADER_TYPE_GBUFFER_LIGHT_POINT 13
#define ZR_SHADER_TYPE_GBUFFER_LIGHT_VOLUME 14
#define ZR_SHADER_TYPE_LAST__ 14

#define ZR_MAX_BATCH_SIZE 100

#include "zr_draw_types.h"

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

///////////////////////////////////////////////////////////////////////////////////
// Draw Frame types:
///////////////////////////////////////////////////////////////////////////////////
/*
Frame draw data struct:
Draw List:
                                        / ZRDrawGroup mesh, prog, <obj, obj, obj, obj>
            / ZRSceneFrame (game)       | ZRDrawGroup mesh, prog, <obj, obj, obj, obj>
                                        \ ZRDrawGroup mesh, prog, <obj, obj, obj, obj>
ZRViewFrame - ZRSceneFrame (view model) - ZRDrawGroups...
            \ ZRSceneFrame (HUD)        - ZRDrawGroups...
*/
#define ZR_PROJECTION_MODE_3D 0
#define ZR_PROJECTION_MODE_IDENTITY 1

// Header before a serialised list of objects
// 
// Output from game scene, input to Renderer

/*

Buffers:
- Draw list -
ZRViewFrame - lists scene frames
	ZRSceneFrame - lists objects
		ZRDrawObj
		ZRDrawObj
		ZRDrawObj
    ZRSceneFrame
		ZRDrawObj
		ZRDrawObj
		ZRDrawObj

- Scratch - 
Stack of random allocations for objects in the scene lists.
Eg text objects will write their strings into here.
Contents is nonsense without the draw frame objects.
*/

struct ZRShader
{
    i32 handle; // considered invalid if this is 0
    i32 drawObjType; // considered invalid if this is 0
    i32 bBatchable;
    char* name;
};

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
    ZRShader* shader;
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
        i32 dataBytes;
        i32 bSkybox;
        i32 bDeferred;
        i32 bIsInteresting;
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
    u32 sentinel;
    f64 prebuildTime;
    i32 bVerbose;
    i32 numScenes;
};

///////////////////////////////////////////////////////////
// Render commands and resources
///////////////////////////////////////////////////////////

#include "assetdb/zr_asset_db.h"

#endif // _ZQF_RENDERER_H
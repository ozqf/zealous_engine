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
#include "ze_common/ze_string_utils.h"

#define ZR_SENTINEL 0xDEADBEEF

#define ZR_TARGET_FRAME_TIME 16.67

// TODO: Changing this breaks things atm
#define ZR_MAX_POINT_LIGHTS_PER_MODEL 8

// If lighting data is transferred via data texture, this is the number of pixels (Vec4s)
// per light.
#define ZR_DATA_PIXELS_PER_LIGHT 3

struct ZRSceneView;
struct ZRShader;

struct ScreenInfo
{
    i32 width;
    i32 height;
    f32 aspectRatio;
};

///////////////////////////////////////////////////////////
// Colours
///////////////////////////////////////////////////////////
union Colour
{
	f32 array[4];
	struct
	{
		f32 red, green, blue, alpha;
	};
    struct
	{
		f32 r, g, b, a;
	};
};

#define COL_U32_WHITE { 1, 1, 1, 1 }
#define COL_U32_BLACK { 0, 0, 0, 1 }
#define COL_U32_RED { 1, 0, 0, 1 }
#define COL_U32_GREEN { 0, 1, 0, 1 }
#define COL_U32_BLUE { 0, 0, 1, 1 }
#define COL_U32_YELLOW { 1, 1, 0, 1 }
#define COL_U32_CYAN { 0, 1, 1, 1 }
#define COL_U32_PURPLE { 1, 0, 1, 1 }

union ColourU32
{
    u8 array[4];
	struct
	{
		u8 red, green, blue, alpha;
	};
    struct
	{
		u8 r, g, b, a;
	};
    struct
    {
        u32 value;
    };
};

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
    f64 gBufferTime;
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

    i32 listBytes;
    i32 dataBytes;

    ZRGroupingStats grouping;
};

///////////////////////////////////////////////////////////
// Imported platform functions
// - passed in at initialisation
///////////////////////////////////////////////////////////
struct ZRPlatform
{
    double (*QueryClock)();
    void* (*Allocate)(i32 numBytes);
    void (*Free)(void* ptr);
};

///////////////////////////////////////////////////////////
// Exported renderer instance
///////////////////////////////////////////////////////////
struct ZRRenderer
{
    i32 isValid;
    ErrorCode (*Init)(i32 scrWidth, i32 scrHeight);
    ZRPerformanceStats (*DrawFrame)(ZEByteBuffer* drawList, ZEByteBuffer* drawData, ScreenInfo scrInfo);
};

///////////////////////////////////////////////////////////
// Exported functions
///////////////////////////////////////////////////////////
extern "C" ZRRenderer ZR_Link(ZRPlatform platform);





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
#define ZR_SHADER_TYPE_LAST__ 11

#define ZR_MAX_BATCH_SIZE 100

#define ZR_UNIQUE_OBJECT_GROUP -1

///////////////////////////////////////////////////////////
// Render scene data types
///////////////////////////////////////////////////////////

#define ZR_DRAWOBJ_TYPE_NONE 0
#define ZR_DRAWOBJ_TYPE_MODEL 1
#define ZR_DRAWOBJ_TYPE_LIGHT 2
#define ZR_DRAWOBJ_TYPE_TEXT 3

#define ZR_DRAWOBJ_STATUS_FREE 0
#define ZR_DRAWOBJ_STATUS_ASSIGNED 1
#define ZR_DRAWOBJ_STATUS_DELETED 2

#define ZR_POINT_LIGHT_DEFAULT_RADIUS 50

union ZRDrawObjUnion
{
	struct
    {
        i32 foo;
    } model;
    struct
    {
		i32 type;
        i32 bCastShadows;
		Vec3 colour;
        Vec4 settings;
    } light;
    struct
    {
        char* text;
        i32 length;
    } text;
};

struct ZRDrawObj
{
    i32 id; // incremental identifier
    i32 status; // check for 'deleted' status if list removal is all done post-frame.
	i32 frameMarker; // Used to mark if the object is relevant to the current frame
    i32 userIndex; // Not used by renderer.
	
	// bounding volume
	Vec3 aabbMin;
	Vec3 aabbMax;
	
    // prefab this object is using
    // This is stored outside the union as it is required by all objects
    // for grouping
    i32 prefabId;
    // shader program used, see above
    i32 program;
	
	i32 parentId;
    Transform t;
    Transform localT;

    // --- discriminated union ---
    // type is also used as part of the grouping Id of this object
	i32 type;
    ZRDrawObjUnion data;
};

struct ZRScene
{
    ZRPlatform platform;

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

///////////////////////////////////////////////////////////
// Quick object initialisation
///////////////////////////////////////////////////////////

static void ZRDrawObj_SetAsModel(ZRScene* s, ZRDrawObj* obj, i32 prefabId)
{
    obj->data = {};
    obj->type = ZR_DRAWOBJ_TYPE_MODEL;
    obj->program = ZR_SHADER_TYPE_BATCHED;
    obj->prefabId = prefabId;
    obj->data.model.foo = YES;
}

static void ZRDrawObj_SetAsPointLight(ZRScene* s, ZRDrawObj* obj, Vec3 colour, f32 radius)
{
    obj->data = {};
    obj->program = ZR_SHADER_TYPE_NONE;
    obj->type = ZR_DRAWOBJ_TYPE_LIGHT;
    obj->prefabId = ZR_UNIQUE_OBJECT_GROUP;
    obj->data.light.colour = colour;
    obj->data.light.settings.x = radius;
}

static void ZRDrawObj_SetAsText(ZRScene* s, ZRDrawObj* obj, char* text)
{
    obj->data = {};
    obj->type = ZR_DRAWOBJ_TYPE_TEXT;
    obj->program = ZR_SHADER_TYPE_TEXT;
    obj->prefabId = ZR_UNIQUE_OBJECT_GROUP;
    obj->data.text.text = text;
    obj->data.text.length = ZE_StrLenNoTerminator(text);
}

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
    //i32 dataSize;
    i32 numScenes;
};

///////////////////////////////////////////////////////////////////////////////////
// Grouping data types
///////////////////////////////////////////////////////////////////////////////////

// Number of pixels (stride) for batch data in data texture.
// 4 for model view, 1 for basic ambient. Rest is for dynamic lights
#define ZR_BATCH_DATA_STRIDE ((ZR_MAX_POINT_LIGHTS_PER_MODEL * ZR_DATA_PIXELS_PER_LIGHT) + 1 + 4)

struct ZRDrawObjLightData
{
    // used to calculate a weight for this light
    f32 distances[ZR_MAX_POINT_LIGHTS_PER_MODEL];

    // used for actual lighting data in shader
    Vec3 pointPositions[ZR_MAX_POINT_LIGHTS_PER_MODEL];
    Vec3 colours[ZR_MAX_POINT_LIGHTS_PER_MODEL];
    Vec4 settings[ZR_MAX_POINT_LIGHTS_PER_MODEL];
};

struct ZRGroupId
{
    i32 objType;
	i32 program;
	i32 prefab;
};

inline static i32 ZRGroupId_Equal(ZRGroupId a, ZRGroupId b)
{
    if (a.objType != b.objType) { return NO; }
    if (a.program != b.program) { return NO; }
    if (a.prefab != b.prefab) { return NO; }
    return YES;
}

inline static ZRGroupId ZRGroupId_Set(i32 objType, i32 program, i32 prefab)
{
    ZRGroupId id;
    id.objType = objType;
    id.program = program;
    id.prefab = prefab;
    return id;
}

struct ZRDrawGroup
{
	// The prefab these objects are grouped on
	//i32 prefab;
    ZRGroupId id;
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

///////////////////////////////////////////////////////////
// Render commands and resources
///////////////////////////////////////////////////////////

struct MeshData
{
	u32 numVerts;

	f32* verts;
	f32* uvs;
    f32* normals;    
};

struct ZRShader
{
    i32 handle; // considered invalid if this is 0
    i32 drawObjType; // considered invalid if this is 0
    i32 bBatchable;
    char* name;
};

// Hold memory and handle to a texture
struct ZRDataTexture
{
    u32 handle;
    i32 bIs1D;
    i32 width;
    i32 height;
    Vec4* mem;
    //Point cursor;
    i32 cursor;
    i32 numBytes;
};

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

///////////////////////////////////////////////////////////
// Draw Commands - written by backend, executed by frontend
///////////////////////////////////////////////////////////

/**
 * Contents of string is written behind the command.
 * String must be null terminated
 * command bytes == size of cmd struct + string itself
 */
#define ZR_TEXT_ALIGNMENT_TOP_RIGHT 0
struct ZRDrawCmd_Text
{
    Vec3 origin;
    i32 numChars; // includes null terminator
    f32 charSize;
    f32 aspectRatio;
    i32 offsetToString;
    i32 alignmentMode;
};

// Scene type

#include "zr_scene.h"

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
	M4x4_Scale(model->cells, modelT->scale.x, modelT->scale.y, modelT->scale.z);
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

////////////////////////////////////////////////////////////////////
// Projection
////////////////////////////////////////////////////////////////////

internal void M4x4_SetProjection(f32* m, f32 prjNear, f32 prjFar, f32 prjLeft, f32 prjRight, f32 prjTop, f32 prjBottom)
{
    m[0] = (2 * prjNear) / (prjRight - prjLeft);
	m[4] = 0;
	m[8] = (prjRight + prjLeft) / (prjLeft - prjRight);
	m[12] = 0;
	
	m[1] = 0;
	m[5] = (2 * prjNear) / (prjTop - prjBottom);
	m[9] = (prjTop + prjBottom) / (prjTop - prjBottom);
	m[13] = 0;
	
	m[2] = 0;
	m[6] = 0;
	m[10] = -(prjFar + prjNear) / (prjFar - prjNear);
	m[14] = (-2 * prjFar * prjNear) / (prjFar - prjNear);
	
	m[3] = 0;
	m[7] = 0;
	m[11] = -1;
	m[15] = 0;
}

internal void M4x4_SetOrthoProjection(f32* m, f32 left, f32 right, f32 top, f32 bottom, f32 prjNear, f32 prjFar)
{
    #if 1
    M4x4_SetToIdentity(m);
    m[0] = 2 / (right - left);
    m[5] = 2 / (top - bottom);
    m[10] = -2 / (prjFar - prjNear);

    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(prjFar + prjNear) / (prjFar - prjNear);
    m[15] = 1;
    #endif
    #if 0
    m[0] = 2; 
    m[5] = 2;
    m[10] = -0.22f;

    m[14] = -1.22f;
    m[15] = 1;
    #endif
}

inline void COM_SetupOrthoProjection(f32* m)
{
	//M4x4_SetOrthoProjection(m, -1, 1, 1, -1, 0.1f, 20.f);
	float size = 40;
	M4x4_SetOrthoProjection(m, -40, size, size, -size, 0.1f, 60.f);
}

inline void COM_Setup3DProjection(
	f32* m4x4,
	i32 fov,
	f32 prjScaleFactor,
	f32 prjNear,
	f32 prjFar,
	f32 aspectRatio)
{
	if (fov <= 0) { fov = 90; }
	M4x4_SetToIdentity(m4x4);
	
	f32 prjLeft = -prjScaleFactor * aspectRatio;
	f32 prjRight = prjScaleFactor * aspectRatio;
	f32 prjTop = prjScaleFactor;
	f32 prjBottom = -prjScaleFactor;

	M4x4_SetProjection(
		m4x4, prjNear, prjFar, prjLeft, prjRight, prjTop, prjBottom);
}

inline void COM_SetupDefault3DProjection(
	f32* m4x4, f32 aspectRatio)
{
	//COM_Setup3DProjection(m4x4, 90, 0.5f, 1.0f, 1000.0f, aspectRatio);
	COM_Setup3DProjection(m4x4, 90, 0.07f, 0.1f, 1000.0f, aspectRatio);
}

////////////////////////////////////////////////////////////////////
// convert homogeneous (-1 to 1) coords to 0 to 1 uv coords
// (for shadow map sampling)
////////////////////////////////////////////////////////////////////
inline void M4x4_HomogeneousToUV(f32* m)
{
	m[0] = 0.5f;
	m[1] = 0;
	m[2] = 0;
	m[3] = 0;

	m[4] = 0;
	m[5] = 0.5f;
	m[6] = 0;
	m[7] = 0;

	m[8] = 0;
	m[9] = 0;
	m[10] = 0.5f;
	m[11] = 0;

	m[12] = 0.5f;
	m[13] = 0.5f;
	m[14] = 0.5f;
	m[15] = 1;
}

#endif // _ZQF_RENDERER_H
#ifndef ZR_DRAW_TYPES_H
#define ZR_DRAW_TYPES_H

#include "ze_common/ze_common.h"
#include "ze_common/ze_hash.h"
#include "ze_common/ze_string_utils.h"

///////////////////////////////////////////////////////////
// Render scene data types
///////////////////////////////////////////////////////////

#define ZR_DRAWOBJ_TYPE_NONE 0
#define ZR_DRAWOBJ_TYPE_MESH 1
#define ZR_DRAWOBJ_TYPE_POINT_LIGHT 2
#define ZR_DRAWOBJ_TYPE_DIRECT_LIGHT 3
#define ZR_DRAWOBJ_TYPE_TEXT 4
#define ZR_DRAWOBJ_TYPE_BILLBOARD 5
#define ZR_DRAWOBJ_TYPE_PREFAB 6

#define ZR_DRAWOBJ_STATUS_FREE 0
#define ZR_DRAWOBJ_STATUS_ASSIGNED 1
#define ZR_DRAWOBJ_STATUS_DELETED 2

#define ZR_POINT_LIGHT_DEFAULT_RADIUS 50

#define ZR_UNIQUE_OBJECT_GROUP -1

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

struct ZRMaterial
{
	char* name;
    i32 index;
	i32 diffuseTexIndex;   	    // default checkerboard 32x32
	i32 emissionTexIndex;		    // default black 16x16
    //i32 normalTexHandle;		// default blue 16x16
	//i32 occulusionTexHandle;	// default black 16x16
	//i32 specularTexHandle;		// default black 16x16
	//Colour baseColour;			// default white
};

struct ZRDrawObjData
{
    i32 type;
    union
    {
        struct
        {
            i32 meshIndex;
            i32 materialIndex;
        } model;
        struct
        {
            i32 prefabId;
        } prefab;
        struct
        {
            i32 frame;
        } billboard;
        struct
        {
            i32 bCastShadows;
	    	Colour colour;
            f32 multiplier;
            f32 range;
        } pointLight;
        struct
        {
            i32 bCastShadows;
            Colour colour;
            f32 multiplier;
            f32 range;
        } directLight;
        struct
        {
            char* text;
            i32 length;
        } text;
    };
};

struct ZRDrawObj
{
    Transform t;
    u32 hash;
    ZRDrawObjData data;
    u32 CalcHash()
    {
        hash = ZE_Hash_djb2_Fixed((u8*)&this->data, sizeof(ZRDrawObjData));
        return hash;
    }
};

struct ZRSceneObj
{
    i32 id; // incremental identifier
    i32 status; // check for 'deleted' status if list removal is all done post-frame.
	i32 frameMarker; // Used to mark if the object is relevant to the current frame
    i32 userIndex; // Not used by renderer.
	
	// bounding volume
	Vec3 aabbMin;
	Vec3 aabbMax;
	
    Transform localT;


	i32 parentId;
};

///////////////////////////////////////////////////////////
// Quick object initialisation
///////////////////////////////////////////////////////////

// static void ZRDrawObj_SetAsPrefab(ZRScene* s, ZRDrawObj* obj, i32 prefabId)
// {
//     obj->data = {};
//     obj->data.type = ZR_DRAWOBJ_TYPE_PREFAB;
//     obj->data.prefab.prefabId = prefabId;
// }

static void ZRDrawObj_SetAsMesh(
    ZRDrawObj* obj, i32 meshIndex, i32 materialIndex)
{
    obj->data = {};
    obj->data.type = ZR_DRAWOBJ_TYPE_MESH;
    obj->data.model.meshIndex = meshIndex;
    obj->data.model.materialIndex = materialIndex;
}

static void ZRDrawObj_SetAsPointLight(
    ZRDrawObj* obj, Colour colour, f32 multiplier, f32 radius)
{
    obj->data = {};
    obj->data.type = ZR_DRAWOBJ_TYPE_POINT_LIGHT;
    obj->data.pointLight.colour = colour;
    obj->data.pointLight.multiplier = multiplier;
    obj->data.pointLight.range = radius;
}

static void ZRDrawObj_SetAsDirectLight(
    ZRDrawObj* obj, Colour colour, f32 multiplier, f32 radius)
{
    obj->data = {};
    obj->data.prefab.prefabId = ZR_UNIQUE_OBJECT_GROUP;
    obj->data.directLight.colour = colour;
    obj->data.directLight.multiplier = multiplier;
    obj->data.directLight.range = radius;
    // M3x3_SetToIdentity(obj->t.rotation.cells);
    // M3x3_RotateX(obj->t.rotation.cells, pitch);
    // M3x3_RotateX(obj->t.rotation.cells, yaw);
}

static void ZRDrawObj_SetAsText(ZRDrawObj* obj, char* text)
{
    obj->data = {};
    obj->data.type = ZR_DRAWOBJ_TYPE_TEXT;
    obj->data.text.text = text;
    obj->data.text.length = ZE_StrLenNoTerminator(text);
}


#endif // ZR_DRAW_TYPES_H
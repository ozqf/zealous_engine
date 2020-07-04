#ifndef ZR_DRAW_TYPES_H
#define ZR_DRAW_TYPES_H

#include "ze_common/ze_common.h"
#include "ze_common/ze_hash.h"
#include "ze_common/ze_string_utils.h"
#include "ze_common/ze_transform.h"

///////////////////////////////////////////////////////////
// Render scene data types
///////////////////////////////////////////////////////////

#define ZR_DRAWOBJ_TYPE_NONE 0
#define ZR_DRAWOBJ_TYPE_MESH 1
#define ZR_DRAWOBJ_TYPE_POINT_LIGHT 2
#define ZR_DRAWOBJ_TYPE_DIRECT_LIGHT 3
#define ZR_DRAWOBJ_TYPE_TEXT 4
#define ZR_DRAWOBJ_TYPE_BILLBOARD 5
#define ZR_DRAWOBJ_TYPE_PARTICLES 6

#define ZR_DRAWOBJ_STATUS_FREE 0
#define ZR_DRAWOBJ_STATUS_ASSIGNED 1
#define ZR_DRAWOBJ_STATUS_DELETED 2

#define ZR_POINT_LIGHT_DEFAULT_RADIUS 50

#define ZR_UNIQUE_OBJECT_GROUP -1

#define ZR_TEXT_ALIGNMENT_TOP_LEFT 0
#define ZR_TEXT_ALIGNMENT_TOP_RIGHT 1
#define ZR_TEXT_ALIGNMENT_CENTRE 2


// Guestimate of an appropriate char size. screen space is 2 units high
// divide this height into N lines:
#define  ZR_SCREEN_SPACE_HEIGHT 2.f
#define  ZR_TEXT_SCREEN_LINE_COUNT 64.f


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

#define COLOUR_WHITE { 1, 1, 1, 1 }
#define COLOUR_BLACK { 0, 0, 0, 1 }
#define COLOUR_RED { 1, 0, 0, 1 }
#define COLOUR_GREEN { 0, 1, 0, 1 }
#define COLOUR_BLUE { 0, 0, 1, 1 }
#define COLOUR_YELLOW { 1, 1, 0, 1 }
#define COLOUR_CYAN { 0, 1, 1, 1 }
#define COLOUR_PURPLE { 1, 0, 1, 1 }

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

struct ZRParticleDef
{
    f32 duration;
    i32 materialIndex;
    i32 meshIndex;
    i32 billboard;
    Vec3 startScale;
    Vec3 endScale;
    Vec3 pull;
};

struct ZRParticle
{
    Vec3 pos;
    Vec3 prevPos;
    Vec3 velocity;
    Vec3 scale;
    f32 tick;
    i32 bCull;
};

struct ZRParticleEmitter
{
    ZRParticleDef def;
    ZRParticle* particles;
    i32 numParticles;
    i32 maxParticles;
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
            i32 billboard;
        } model;
        // struct
        // {
        //     i32 frame;
        // } billboard;
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
			i32 charTextureIndex;
			Colour colour;
			i32 alignment;
        } text;
        ZRParticleDef particleEmitter;
    };

    void SetAsMesh(i32 meshIndex, i32 materialIndex)
    {
        this->type = ZR_DRAWOBJ_TYPE_MESH;
        this->model.meshIndex = meshIndex;
        this->model.materialIndex = materialIndex;
    }

    void SetAsPointLight(Colour colour, f32 multiplier, f32 radius)
    {
        this->type = ZR_DRAWOBJ_TYPE_POINT_LIGHT;
        this->pointLight.colour = colour;
        this->pointLight.multiplier = multiplier;
        this->pointLight.range = radius;
    }
    
    void SetAsDirectLight(Colour colour, f32 multiplier, f32 radius)
    {
        this->type = ZR_DRAWOBJ_TYPE_DIRECT_LIGHT;
        this->directLight.colour = colour;
        this->directLight.multiplier = multiplier;
        this->directLight.range = radius;
    }

	void SetAsText(char* chars, i32 texIndex, Colour colour, i32 alignment)
	{
		this->type = ZR_DRAWOBJ_TYPE_TEXT;
		this->text.text = chars;
		this->text.length = ZE_StrLenNoTerminator(chars) + 1;
		if (texIndex < 0)
		{
			this->text.charTextureIndex = -1;
		}
		this->text.colour = colour;
		this->text.alignment = alignment;
	}
};

struct ZRDrawObj
{
    Transform t;
    // For interpolation
    Vec3 prevPos;
    // quick tag for finding objects for debugging
    i32 debugTag;
    // hash of data union, used to identify objects which are
    // similar and could be batched
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

static ZRDrawObj* ZRDrawObj_InitInPlace(u8** ptr)
{
    ZRDrawObj* obj = (ZRDrawObj*)*ptr;
    *ptr += sizeof(ZRDrawObj);
    *obj = {};
    Transform_SetToIdentity(&obj->t);
    return obj;
}

static f32 ZR_CharScreenSizeDefault()
{
	return ZR_SCREEN_SPACE_HEIGHT / ZR_TEXT_SCREEN_LINE_COUNT;
}

static f32 ZR_CalcCharHalfWidth(f32 charSize, f32 aspectRatio)
{
	return (charSize * (aspectRatio - 1.f)) / 2.f;
}

static f32 ZR_CalcCharHalfHeight(f32 charSize)
{
	return charSize / 2.f;
}

#endif // ZR_DRAW_TYPES_H
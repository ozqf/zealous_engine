/*
Zealous Engine public header
*/
#ifndef ZENGINE_H
#define ZENGINE_H

#include "ze_common.h"

///////////////////////////////////////////////////////////
// Embedded assets
///////////////////////////////////////////////////////////
#define FALLBACK_TEXTURE_NAME "fallback_texture"
#define FALLBACK_CHARSET_TEXTURE_NAME "fallback_charset"
#define FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME "fallback_charset_semi_transparent"

///////////////////////////////////////////////////////////
// Colours
///////////////////////////////////////////////////////////
union ColourF32
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
// Asset data types
///////////////////////////////////////////////////////////

#define ZE_ASSET_TYPE_NONE 0
#define ZE_ASSET_TYPE_TEXTURE 1
#define ZE_ASSET_TYPE_MESH 2
#define ZE_ASSET_TYPE_MATERIAL 2

struct ZRAsset
{
    i32 id;
    i32 index;
    i32 type;
    // On GPU - handles are set
    i32 bIsUploaded;
    // Data has changed - needs to be re-uploaded
    i32 bIsDirty;
    char *fileName;
    i32 sentinel;
};

struct ZRMaterial
{
    ZRAsset header;
    i32 programId;
    i32 diffuseTexId;
    i32 emissionTexId;
};

struct ZRTexture
{
    ZRAsset header;
    ColourU32 *data;
    i32 width;
    i32 height;
};

#define VEC3_SIZE = 12
#define VEC2_SIZE = 8

// Currently stores every vertex, no sharing
struct ZRMeshData
{
    u32 numVerts;
    // dynamic mesh may have more capacity.
    u32 maxVerts;

    f32 *verts;
    f32 *uvs;
    f32 *normals;

    Vec3 *GetVert(i32 i) { return (Vec3 *)(verts + (i * 3)); }

    void Clear()
    {
        this->numVerts = 0;
    }

    void AddTri(
        Vec3 v0, Vec3 v1, Vec3 v2,
        Vec2 uv0, Vec2 uv1, Vec2 uv2,
        Vec3 n0, Vec3 n1, Vec3 n2)
    {
        i32 i = this->numVerts;
        this->numVerts += 1;
        // step to 
        i32 vertStride = sizeof(f32) * 3 * i;
        i32 uvStride = sizeof(f32) * 2 * i;
        Vec3* vert = (Vec3*)((u8*)verts + vertStride);
        Vec2* uv = (Vec2*)((u8*)uvs + uvStride);
        Vec3* normal = (Vec3*)((u8*)normals + vertStride);
        vert[0] = v0;
        vert[1] = v1;
        vert[2] = v2;
        uv[0] = uv0;
        uv[1] = uv1;
        uv[2] = uv2;
        normal[0] = n0;
        normal[1] = n1;
        normal[2] = n2;
    }

    void AddVert(
        Vec3 vert,
        Vec2 uv,
        Vec3 normal)
    {
        if (this->numVerts >= this->maxVerts) { return; }
        i32 i = this->numVerts;
        this->numVerts += 1;
        // step to
        i32 vertStride = sizeof(Vec3) * i;
        i32 uvStride = sizeof(Vec2) * i;
        Vec3 *vertPtr = (Vec3 *)((u8 *)verts + vertStride);
        Vec2 *uvPtr = (Vec2 *)((u8 *)uvs + uvStride);
        Vec3 *normalPtr = (Vec3 *)((u8 *)normals + vertStride);
        vertPtr[0] = vert;
        uvPtr[0] = uv;
        normalPtr[0] = normal;
    }

    i32 MeasureBytes()
    {
        i32 bytes = 0;
        const i32 v3size = sizeof(f32) * 3;
        const i32 v2size = sizeof(f32) * 2;
        bytes += v3size * numVerts;
        bytes += v2size * numVerts;
        bytes += v3size * numVerts;
        return bytes;
    }

    i32 CopyData(ZRMeshData original)
    {
        if (original.numVerts > maxVerts)
        {
            printf("No space to copy mesh! %d verts have %d\n",
                   original.numVerts, maxVerts);
            return ZE_ERROR_NO_SPACE;
        }
        numVerts = original.numVerts;
        const i32 numVertBytes = (sizeof(f32) * 3) * numVerts;
        const i32 numUVSBytes = (sizeof(f32) * 2) * numVerts;
        printf("Copying %d verts (%d vert bytes, %d uv bytes)\n",
               numVerts, numVertBytes, numUVSBytes);
        ZE_Copy(verts, original.verts, numVertBytes);
        ZE_Copy(uvs, original.uvs, numUVSBytes);
        ZE_Copy(normals, original.normals, numVertBytes);
        return ZE_ERROR_NONE;
    }

    void PrintVerts()
    {
        printf("--- %d verts ---\n", numVerts);
        f32 *cursor = verts;
        for (u32 i = 0; i < numVerts; ++i)
        {
            printf("%d: %.3f, %.3f, %.3f\n", i, cursor[0], cursor[1], cursor[2]);
            cursor += 3;
        }
    }
};

struct ZRMeshAsset
{
    ZRAsset header;
    ZRMeshData data;
};

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
#define ZR_DRAWOBJ_TYPE_SPRITE 7

#define ZR_DRAWOBJ_STATUS_FREE 0
#define ZR_DRAWOBJ_STATUS_ASSIGNED 1
#define ZR_DRAWOBJ_STATUS_DELETED 2

struct ZRDrawObjData
{
    i32 type;
    union
    {
        struct
        {
            i32 meshId;
            i32 materialId;
            i32 billboard;
        } model;
        struct
        {
            u32 frameId;
        } sprite;
        struct
        {
            i32 bCastShadows;
            ColourU32 colour;
            f32 multiplier;
            f32 range;
        } pointLight;
        struct
        {
            i32 bCastShadows;
            ColourU32 colour;
            f32 multiplier;
            f32 range;
        } directLight;
        struct
        {
            char *text;
            i32 length;
            i32 linesPerScreen;
            i32 charTextureId;
            ColourU32 colour;
            ColourU32 bgColour;
            i32 alignment;
        } text;
    };

    void SetAsMesh(i32 meshId, i32 materialId)
    {
        this->type = ZR_DRAWOBJ_TYPE_MESH;
        this->model.meshId = meshId;
        this->model.materialId = materialId;
    }

    void SetAsSprite(u32 spriteFrameId)
    {
        this->type = ZR_DRAWOBJ_TYPE_SPRITE;
        this->sprite.frameId = spriteFrameId;
    }

    void SetAsPointLight(ColourU32 colour, f32 multiplier, f32 radius)
    {
        this->type = ZR_DRAWOBJ_TYPE_POINT_LIGHT;
        this->pointLight.colour = colour;
        this->pointLight.multiplier = multiplier;
        this->pointLight.range = radius;
    }

    void SetAsDirectLight(ColourU32 colour, f32 multiplier, f32 radius)
    {
        this->type = ZR_DRAWOBJ_TYPE_DIRECT_LIGHT;
        this->directLight.colour = colour;
        this->directLight.multiplier = multiplier;
        this->directLight.range = radius;
    }

    void SetAsText(char *chars, i32 texId, ColourU32 colour, ColourU32 bgColour, i32 alignment)
    {
        this->type = ZR_DRAWOBJ_TYPE_TEXT;
        this->text.text = chars;
        this->text.length = ZStr_LenNoTerminator(chars) + 1;
        if (texId >= 0)
        {
            this->text.charTextureId = texId;
        }
        else
        {
            this->text.charTextureId = 0;
        }
        this->text.colour = colour;
        this->text.bgColour = bgColour;
        this->text.alignment = alignment;
    }
};

struct ZRDrawObj
{
    Transform t;
    // For interpolation
    Vec3 prevPos;
    // a user Id to tag the object. not used in rasterisation
    zeHandle userTag;
    // hash of data union, used to identify objects which are
    // similar and could be batched
    u32 hash;
    ZRDrawObjData data;
    u32 CalcHash()
    {
        hash = ZE_Hash_djb2_Fixed((u8 *)&this->data, sizeof(ZRDrawObjData));
        return hash;
    }
};

// game functions provided to engine
struct ZGame
{
    void (*Init)();
    void (*Shutdown)();
    void (*Tick)();
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

    ZRTexture *(*AllocTexture)(i32 width, i32 height, char *name);
};

// engine functions provided to game
struct ZEngine
{
    ZSceneManager scenes;
    ZAssetManager assets;
    ZInput input;
    i32 sentinel;
};

#endif // ZENGINE_H

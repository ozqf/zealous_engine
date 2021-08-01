/*
Zealous Engine public header
*/
#ifndef ZENGINE_H
#define ZENGINE_H

#include "ze_common.h"

////////////////////////////////////////////////
// draw types
////////////////////////////////////////////////

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

#define COLOUR_U32_GREY { 155, 155, 155, 155 }
#define COLOUR_U32_GREY_DARK { 200, 200, 200, 200 }

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
};

struct ZRTexture
{
    //char* fileName;
    ZRAsset header;
    // 32 bit pixel data.
    ColourU32 *data;
    i32 width;
    i32 height;
    i32 apiHandle;
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

#endif // ZENGINE_H

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

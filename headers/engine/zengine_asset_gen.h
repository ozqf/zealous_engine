#ifndef ZENGINE_ASSET_GEN_H
#define ZENGINE_ASSET_GEN_H

#include "../zengine.h"

inline i32 TexGen_DecodeBW(
    u8 *source,
    const i32 sourceSize,
    ColourU32 *target,
    const i32 w,
    const i32 h,
    ColourU32 solid,
    ColourU32 empty)
{
    for (i32 i = 0; i < sourceSize; ++i)
    {
        u8 block = source[i];
        for (i32 bit = 0; bit < 8; ++bit)
        {
            ColourU32 *colour = &target[(i * 8) + bit];
            if (block & (1 << bit))
            {
                *colour = solid;
            }
            else
            {
                *colour = empty;
            }
        }
    }
    return ZE_ERROR_NONE;
}

//////////////////////////////////////////////////////
// geometry
//////////////////////////////////////////////////////
inline void ZGen_AddSriteGeoXY(
    ZRMeshData* meshData, Vec3 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax, f32 radians)
{
    //printf("Add sprite geo, rotation %.3f degrees\n", radians * RAD2DEG);
    // build verts at 0,0,0, apply rotation and move into position
    Vec3 verts[6];
    verts[0] = { -size.x, -size.y, 0 };
    verts[1] = { size.x, -size.y, 0 };
    verts[2] = { size.x, size.y, 0 };

    verts[3] = { -size.x, -size.y, 0 };
    verts[4] = { size.x, size.y, 0 };
    verts[5] = { -size.x, size.y, 0 };

    M3x3_CREATE(rotM)
    M3x3_RotateZ(rotM.cells, radians);
    Vec3_MultiplyByM3x3(&verts[0], rotM.cells);
    Vec3_MultiplyByM3x3(&verts[1], rotM.cells);
    Vec3_MultiplyByM3x3(&verts[2], rotM.cells);

    Vec3_MultiplyByM3x3(&verts[3], rotM.cells);
    Vec3_MultiplyByM3x3(&verts[4], rotM.cells);
    Vec3_MultiplyByM3x3(&verts[5], rotM.cells);

     Vec3_AddTo(&verts[0], pos);
     Vec3_AddTo(&verts[1], pos);
     Vec3_AddTo(&verts[2], pos);

     Vec3_AddTo(&verts[3], pos);
     Vec3_AddTo(&verts[4], pos);
     Vec3_AddTo(&verts[5], pos);

    meshData->AddVert(verts[0], {uvMin.x, uvMin.y}, {0, 0, -1});
    meshData->AddVert(verts[1], {uvMax.x, uvMin.y}, {0, 0, -1});
    meshData->AddVert(verts[2], {uvMax.x, uvMax.y}, {0, 0, -1});

    meshData->AddVert(verts[3], {uvMin.x, uvMin.y}, {0, 0, -1});
    meshData->AddVert(verts[4], {uvMax.x, uvMax.y}, {0, 0, -1});
    meshData->AddVert(verts[5], {uvMin.x, uvMax.y}, {0, 0, -1});

    // meshData->AddVert({pos.x + -size.x, pos.y + -size.y, pos.z}, {uvMin.x, uvMin.y}, {0, 0, -1});
    // meshData->AddVert({pos.x + size.x, pos.y + -size.y, pos.z}, {uvMax.x, uvMin.y}, {0, 0, -1});
    // meshData->AddVert({pos.x + size.x, pos.y + size.y, pos.z}, {uvMax.x, uvMax.y}, {0, 0, -1});

    // meshData->AddVert({pos.x + -size.x, pos.y + -size.y, pos.z}, {uvMin.x, uvMin.y}, {0, 0, -1});
    // meshData->AddVert({pos.x + size.x, pos.y + size.y, pos.z}, {uvMax.x, uvMax.y}, {0, 0, -1});
    // meshData->AddVert({pos.x + -size.x, pos.y + size.y, pos.z}, {uvMin.x, uvMax.y}, {0, 0, -1});
}

//////////////////////////////////////////////////////
// textures
//////////////////////////////////////////////////////
inline void ZGen_SetPixel(
    ZRTexture* tex, ColourU32 colour, i32 x, i32 y)
{
    if (tex == NULL) { return; }
    ColourU32* pixels = tex->data;
    i32 w = tex->width;
    i32 h = tex->height;
    if (pixels == NULL) { return; }
    if (x < 0 || x >= w) { return; }
    if (y < 0 || y >= h) { return; }
    i32 i = ZE_2D_INDEX(x, y, w);
    pixels[i] = colour;
}

inline void ZGen_DrawLine(
    ZRTexture *tex, ColourU32 colour, Point2 a, Point2 b)
{
    if (tex == NULL) { return; }
    ColourU32 *pixels = tex->data;
    i32 w = tex->width;
    i32 h = tex->height;
}

inline void ZGen_FillTextureRect(
    ZRTexture *tex, ColourU32 colour, Point2 topLeft, Point2 size)
{
    if (tex == NULL) { return; }
    ColourU32 *pixels = tex->data;
    i32 w = tex->width;
    i32 h = tex->height;
    if (pixels == NULL) { return; }

    Point2 min = topLeft;
    Point2 max = topLeft;
    max.x += size.x;
    max.y += size.y;
    // cap positions
    if (min.x < 0) { min.x = 0; }
    if (min.y < 0) { min.y = 0; }
    if (max.x > w) { max.x = w - 1; }
    if (max.y > h) { max.y = h - 1; }

    for (i32 y = min.y; y < max.y; ++y)
    {
        for (i32 x = min.x; x < max.x; ++x)
        {
            i32 i = ZE_2D_INDEX(x, y, w);
            pixels[i] = colour;
        }
    }
}

inline void ZGen_FillTexture(ZRTexture *tex, ColourU32 colour)
{
    if (tex == NULL) { return; }
    ColourU32 *pixels = tex->data;
    i32 w = tex->width;
    i32 h = tex->height;
    if (pixels == NULL) { return; }
    i32 numPixels = w * h;
    for (i32 i = 0; i < numPixels; ++i)
    {
        pixels[i] = colour;
    }
}

inline void ZGen_FillTexturePixelChequer(ZRTexture *tex, ColourU32 odd, ColourU32 even)
{
    if (tex == NULL) { return; }
    ColourU32 *pixels = tex->data;
    i32 w = tex->width;
    i32 h = tex->height;
    if (pixels == NULL) { return; }
    i32 numPixels = w * h;
	for (i32 y = 0; y < h; ++y)
	{
		for (i32 x = 0; x < w; ++x)
		{
			i32 bEvenX = (x % 2 == 0);
			i32 bEvenY = (y % 2 == 0);
			i32 i = ZE_2D_INDEX(x, y, w);
			if (bEvenX == bEvenY)
			{
				pixels[i] = even;
			}
			else
			{
				pixels[i] = odd;
			}
		}
	}
}

#endif // ZENGINE_ASSET_GEN_H

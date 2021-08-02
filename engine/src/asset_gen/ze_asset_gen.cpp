/*
Provides functions for creating/modifying assets
programmatically
*/
#include "../../internal_headers/zengine_internal.h"

ze_external i32 TexGen_DecodeBW(
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
ze_external void ZGen_AddSriteGeoXY(
    ZRMeshData* meshData, Vec2 pos, Vec2 size, Vec2 uvMin, Vec2 uvMax)
{
    meshData->AddVert({pos.x + -size.x, pos.y + -size.y, 0}, {uvMin.x, uvMin.y}, {0, 0, -1});
    meshData->AddVert({pos.x + size.x, pos.y + -size.y, 0}, {uvMax.x, uvMin.y}, {0, 0, -1});
    meshData->AddVert({pos.x + size.x, pos.y + size.y, 0}, {uvMax.x, uvMax.y}, {0, 0, -1});

    meshData->AddVert({pos.x + -size.x, pos.y + -size.y, 0}, {uvMin.x, uvMin.y}, {0, 0, -1});
    meshData->AddVert({pos.x + size.x, pos.y + size.y, 0}, {uvMax.x, uvMax.y}, {0, 0, -1});
    meshData->AddVert({pos.x + -size.x, pos.y + size.y, 0}, {uvMin.x, uvMax.y}, {0, 0, -1});
}

//////////////////////////////////////////////////////
// textures
//////////////////////////////////////////////////////
ze_external void ZGen_SetPixel(
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

ze_external void ZGen_DrawLine(
    ZRTexture *tex, ColourU32 colour, Point2 a, Point2 b)
{
    if (tex == NULL) { return; }
    ColourU32 *pixels = tex->data;
    i32 w = tex->width;
    i32 h = tex->height;
}

ze_external void ZGen_FillTextureRect(
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

ze_external void ZGen_FillTexture(ZRTexture *tex, ColourU32 colour)
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

ze_external void ZGen_Init()
{

}

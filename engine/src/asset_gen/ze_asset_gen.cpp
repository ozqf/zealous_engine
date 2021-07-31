#include "../../internal_headers/zengine_internal.h"

ze_external void ZGen_Init()
{

}

ze_external void ZGen_SetPixel(
    ColourU32 *pixels, i32 w, i32 h, ColourU32 colour, i32 x, i32 y)
{
    if (pixels == NULL) { return; }
    if (x < 0 || x >= w) { return; }
    if (y < 0 || y >= h) { return; }
    i32 i = ZE_2D_INDEX(x, y, w);
    printf("Set pixel at %d\n", i);
    pixels[i] = colour;
}

ze_external void ZGen_DrawLine(
    ColourU32 *pixels, i32 w, i32 h, ColourU32 colour, Point2 a, Point2 b)
{

}

ze_external void ZGen_FillTextureRect(
    ColourU32 *pixels, i32 w, i32 h, ColourU32 colour, Point2 topLeft, Point2 size)
{
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

ze_external void ZGen_FillTexture(ColourU32 *pixels, i32 w, i32 h, ColourU32 colour)
{
    if (pixels == NULL) { return; }
    i32 numPixels = w * h;
    for (i32 i = 0; i < numPixels; ++i)
    {
        pixels[i] = colour;
    }
}

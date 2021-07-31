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

ze_external void ZGen_FillTexture(ColourU32 *pixels, i32 w, i32 h, ColourU32 colour)
{
    if (pixels == NULL) { return; }
    i32 numPixels = w * h;
    for (i32 i = 0; i < numPixels; ++i)
    {
        pixels[i] = colour;
    }
}

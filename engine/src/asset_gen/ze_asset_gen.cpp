#include "../../internal_headers/zengine_internal.h"

ze_external void ZGen_Init()
{

}

ze_external void ZGen_FillTexture(ColourU32 *pixels, i32 w, i32 h, ColourU32 colour)
{
    i32 numPixels = w * h;
    for (i32 i = 0; i < numPixels; ++i)
    {
        pixels[i] = colour;
    }
}

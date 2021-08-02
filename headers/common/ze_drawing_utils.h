#ifndef ZE_DRAWING_UTILS_H
#define ZE_DRAWING_UTILS_H

#define ZRGL_ASCI_CHARSET_CHARS_WIDE 16

#include "../ze_common.h"

internal void ZEAsciToCharsheetUVs(char c, Vec2* min, Vec2* max)
{
    const f32 stride = 1.f / (f32)ZRGL_ASCI_CHARSET_CHARS_WIDE;

    i32 sheetX = c % ZRGL_ASCI_CHARSET_CHARS_WIDE;
    i32 sheetY = c / ZRGL_ASCI_CHARSET_CHARS_WIDE;

    // Sheet is top -> down but opengl is bottom -> up so flip the Y coord
    sheetY = (16 - 1) - sheetY;
    
    min->x = stride * (f32)sheetX;
    min->y = stride * (f32)sheetY;

    max->x = min->x + stride;
    max->y = min->y + stride;
}

#endif // ZE_DRAWING_UTILS_H

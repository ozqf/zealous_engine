#ifndef ZR_GROUPS_H
#define ZR_GROUPS_H

#include "../zqf_renderer.h"
#include "../ze_common/ze_byte_buffer.h"

extern "C" inline Point ZR_IndexToPixel(int index, int imageWidth);
extern "C" inline i32 ZR_PixelToIndex(i32 x, i32 y, int imageWidth);

extern "C" ZRSceneView* ZR_BuildDrawGroups(
    ZRDrawObj* objects,
    i32 numObjects,
    ZEByteBuffer* scratch,
    ZRGroupingStats* stats);

extern "C" void ZR_WriteGroupsToTextureByIndex(
    ZRDrawObj* objects, i32 numObjects,
    Transform* camT, ZRSceneView* groups,
    ZRDataTexture* tex);

#endif // ZR_GROUPS_H
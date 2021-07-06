#ifndef ZR_GROUPS_H
#define ZR_GROUPS_H

#include "zrgl.h"
#include "../../headers/common/ze_byte_buffer.h"

/*
Scenes are broken down to groups, combo of mesh, proj, material + list of transforms:
Draw List:
                                        / ZRDrawGroup mesh, prog, <obj, obj, obj, obj>
            / ZRSceneFrame (game)       | ZRDrawGroup mesh, prog, <obj, obj, obj, obj>
                                        \ ZRDrawGroup mesh, prog, <obj, obj, obj, obj>
ZRViewFrame - ZRSceneFrame (view model) - ZRDrawGroups...
            \ ZRSceneFrame (HUD)        - ZRDrawGroups...
*/
///////////////////////////////////////////////////////////////
// Grouping data types
///////////////////////////////////////////////////////////////

// Number of pixels (stride) for batch data in data texture.
// 4 for model view, 1 for basic ambient. Rest is for dynamic lights
#define ZR_BATCH_DATA_STRIDE ((ZR_MAX_POINT_LIGHTS_PER_MODEL * ZR_DATA_PIXELS_PER_LIGHT) + 1 + 4)

struct ZRDrawObjLightData
{
    // used to calculate a weight for this light
    f32 distances[ZR_MAX_POINT_LIGHTS_PER_MODEL];

    // used for actual lighting data in shader
    Vec3 pointPositions[ZR_MAX_POINT_LIGHTS_PER_MODEL];
    Vec3 colours[ZR_MAX_POINT_LIGHTS_PER_MODEL];
    Vec4 settings[ZR_MAX_POINT_LIGHTS_PER_MODEL];
};

///////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////

extern "C" Point2 ZR_IndexToPixel(int index, int imageWidth);
extern "C" i32 ZR_PixelToIndex(i32 x, i32 y, int imageWidth);

extern "C" ZRSceneView* ZR_BuildDrawGroups(
    ZRDrawObj* objects,
    i32 numObjects,
    ZEBuffer* scratch,
    ZRGroupingStats* stats);

extern "C" void ZR_WriteGroupsToTextureByIndex(
    ZRDrawObj* objects, i32 numObjects,
    Transform* camT, ZRSceneView* groups,
    ZRDataTexture* tex);

#endif // ZR_GROUPS_H
#ifndef ZE_SCENE_INTERNAL_H
#define ZE_SCENE_INTERNAL_H

#include "../../internal_headers/zengine_internal.h"

/*
Visual Scene manager
Scene structs are stored in a hash table
scene structs have a blob store of the objects within them.

scenes draw in order, lowest to highest, as isolated passes

*/
#define ZSCENE_PROJECTION_MODE_MANUAL 0
#define ZSCENE_PROJECTION_MODE_3D 1
#define ZSCENE_PROJECTION_MODE_ORTHO 2

struct ZRScene
{
    zeHandle id;
    u32 flags;
    // Tightly packed list of objects
    ZEBlobStore objects;
    ZEBlobStore userStore;
    zeSize userStoreItemSize;
    i32 bDebug;
    zeHandle nextId;
    i32 numObjects;
    i32 maxObjects;

    i32 projectionMode;
    union
    {
        struct
        {
            f32 verticalExtent;
        } presetOrth;
        struct
        {
            f32 fov;
        } preset3d;
        struct
        {
            M4x4 projection;
        } custom;
    } projectionInfo;

    Transform camera;
    // M4x4 projection;
};

ze_external void ZScene_WriteDrawCommands(ZEBuffer *buf, ZRScene *scene);
ze_external void ZScene_InitGrouping();

#endif // ZE_SCENE_INTERNAL_H

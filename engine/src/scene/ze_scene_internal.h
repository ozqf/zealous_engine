#ifndef ZE_SCENE_INTERNAL_H
#define ZE_SCENE_INTERNAL_H

#include "../../internal_headers/zengine_internal.h"

/*
Visual Scene manager
Scene structs are stored in a hash table
scene structs have a blob store of the objects within them.

scenes draw in order, lowest to highest, as isolated passes

*/

struct ZRScene
{
    zeHandle id;
    // Tightly packed list of objects
    ZEBlobStore objects;
    i32 bDebug;
    i32 nextId;
    i32 numObjects;
    i32 maxObjects;

    Transform camera;
    M4x4 projection;
};

ze_external void ZScene_WriteDrawCommands(ZEBuffer *buf, ZRScene *scene);

#endif // ZE_SCENE_INTERNAL_H

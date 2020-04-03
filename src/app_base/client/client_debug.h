#ifndef CLIENT_DEBUG_H
#define CLIENT_DEBUG_H

#include "../../voxel_world/voxel_world.h"

static void CLDebug_MakeVoxelWorld()
{
    VWChunk* chunk;
    VWError err = VW_AllocChunk(8, &chunk);
    if (err != 0)
    {
        printf("Error %d creating VWChunk\n", err);
        return;
    }
    printf("Made VWChunk size %d with %d blocks\n", chunk->size, chunk->numBlocks);
}

#endif // CLIENT_DEBUG_H
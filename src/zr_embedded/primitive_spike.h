
#include "../ze_common/ze_common.h"

#define SPIKE_SIZE 1.0f

internal f32 g_prim_spikeVerts[] =
{
    // BASE
    // face 0
    -SPIKE_SIZE, -SPIKE_SIZE,  SPIKE_SIZE,
     SPIKE_SIZE, -SPIKE_SIZE,  SPIKE_SIZE,
     0,          SPIKE_SIZE,  SPIKE_SIZE,

    // SIDES
    // face 1
     SPIKE_SIZE, -SPIKE_SIZE,  SPIKE_SIZE,
     0,  0, -SPIKE_SIZE,
     0,  SPIKE_SIZE,  SPIKE_SIZE,
    // face 2
    -SPIKE_SIZE, -SPIKE_SIZE,  SPIKE_SIZE,
     0,  SPIKE_SIZE,  SPIKE_SIZE,
     0,  0, -SPIKE_SIZE,
    // face 3
    -SPIKE_SIZE, -SPIKE_SIZE,  SPIKE_SIZE,
     0,          0,           -SPIKE_SIZE,
     SPIKE_SIZE, -SPIKE_SIZE,  SPIKE_SIZE
};

internal f32 g_prim_spikeUVs[] =
{
    // face 0
    0, 0,
    1, 0,
    0.5, 1,
    // face 1
    0, 0,
    1, 0,
    0.5, 1,
    // face 2
    0, 0,
    1, 0,
    0.5, 1,
    // face 3
    0, 0,
    1, 0,
    0.5, 1,
    // face 4
    0, 1,
    0.5, 0,
    1, 1,
};

internal f32 g_prim_spikeNormals[] =
{
    // TOP
    // face 0
    -1,  0,  1,
     1,  0,  1,
     0,  1,  0,
    // face 1
    1, 0, 1,
    1, 0, -1,
    0, 1, 0,
    // face 2
    1, 0, -1,
    -1, 0, -1,
    0, 1, 0,
    // face 3
    -1, 0, -1,
    -1, 0, 1,
    0, 1, 0

    // BOTTOM
    // face 4
    -1,  0,  1,
     0,  -1,  0,
     1,  0,  1,
};


internal MeshData g_meshSpike = {
    12,
    g_prim_spikeVerts,
    g_prim_spikeUVs,
    g_prim_spikeNormals
};

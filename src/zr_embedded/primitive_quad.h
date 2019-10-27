
#include "../ze_common/ze_common.h"

internal f32 g_prim_quadVerts[] =
{
	-0.5, -0.5,  0,
	 0.5, -0.5,  0,
	 0.5,  0.5,  0,

	-0.5, -0.5,  0,
	 0.5,  0.5,  0,
	-0.5,  0.5,  0
};

internal f32 g_prim_quadUVs[] =
{
	0, 0,
	1, 0,
	1, 1,

	0, 0,
	1, 1,
	0, 1
};

internal f32 g_prim_quadNormals[] =
{
	-1, -1,  0,
	 1, -1,  0,
	 1,  1,  0,

	-1, -1,  0,
	 1,  1,  0,
	-1,  1,  0
};

internal MeshData g_meshPrimitive_quad = {
    6,
    g_prim_quadVerts,
    g_prim_quadUVs

};


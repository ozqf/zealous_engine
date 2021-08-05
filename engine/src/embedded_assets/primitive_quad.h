
#include "../../../headers/ze_common.h"

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
	#if 0
	-1, -1,  0,
	 1, -1,  0,
	 1,  1,  0,

	-1, -1,  0,
	 1,  1,  0,
	-1,  1,  0
	#endif
	#if 1
	0,  0,  -1,
	0,  0,  -1,
	0,  0,  -1,

	0,  0,  -1,
	0,  0,  -1,
	0,  0,  -1
	#endif
	#if 0
	0,  1,  0,
	0,  1,  0,
	0,  1,  0,

	0,  1,  0,
	0,  1,  0,
	0,  1,  0
	#endif
};

internal ZRMeshData g_meshPrimitive_quad = {
    6,
	6,
    g_prim_quadVerts,
    g_prim_quadUVs,
	g_prim_quadNormals
};


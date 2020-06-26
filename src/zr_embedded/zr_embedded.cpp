#ifndef ZR_EMBEDDED_CPP
#define ZR_EMBEDDED_CPP

#include "zr_embedded.h"

#include "primitive_cube.h"
#include "primitive_octahedron.h"
#include "primitive_quad.h"
#include "primitive_spike.h"

#include "bw_charset.h"

#include "zr_tex_gen.h"

extern "C" MeshData* ZR_Embed_Cube()
{
    return &g_meshCube;
}

extern "C" MeshData* ZR_Embed_InverseCube()
{
    return &g_meshInverseCube;
}

extern "C" MeshData* ZR_Embed_Quad()
{
    return &g_meshPrimitive_quad;
}

extern "C" MeshData* ZR_Embed_Spike()
{
    return &g_meshSpike;
}

extern "C" MeshData* ZR_Embed_Octahedron()
{
    return &g_meshOctahedron;
}

extern "C" BWImage ZR_Embed_Charset()
{
	BWImage img;
	img.name = bw_charset_name;
	img.bytes = bw_charset_bytes;
	img.numBytes = bw_charset_num_bytes;
	img.w = bw_charset_width;
	img.h = bw_charset_height;
	return img;
}

#endif // ZR_EMBEDDED_CPP
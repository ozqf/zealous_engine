#ifndef ZR_EMBEDDED_H
#define ZR_EMBEDDED_H

#include "../ze_common/ze_common.h"
#include "../zqf_renderer.h"

extern "C" MeshData* ZR_Embed_Cube();
extern "C" MeshData* ZR_Embed_InverseCube();
extern "C" MeshData* ZR_Embed_Quad();
extern "C" MeshData* ZR_Embed_Spike();
extern "C" MeshData* ZR_Embed_Octahedron();

#endif // ZR_EMBEDDED_H
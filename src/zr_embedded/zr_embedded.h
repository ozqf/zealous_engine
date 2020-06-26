#ifndef ZR_EMBEDDED_H
#define ZR_EMBEDDED_H

#include "../ze_common/ze_common.h"
#include "../zqf_renderer.h"

extern "C" MeshData* ZR_Embed_Cube();
extern "C" MeshData* ZR_Embed_InverseCube();
extern "C" MeshData* ZR_Embed_Quad();
extern "C" MeshData* ZR_Embed_Spike();
extern "C" MeshData* ZR_Embed_Octahedron();

extern "C" i32 TexGen_BytesFor32BitImage(i32 width, i32 height);
extern "C" i32 TexGen_MeasureBWImage(i32 width, i32 height);
extern "C" void TexGen_SetRGBA(ColourU32* pixels, i32 width, i32 height, ColourU32 colour);

#endif // ZR_EMBEDDED_H
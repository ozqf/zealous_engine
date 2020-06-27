#ifndef ZR_EMBEDDED_H
#define ZR_EMBEDDED_H

#include "../ze_common/ze_common.h"
#include "../zqf_renderer.h"

struct BWImage
{
	char* name;
	u8* bytes;
	i32 numBytes;
	i32 w;
	i32 h;
};

extern "C" MeshData* ZR_Embed_Cube();
extern "C" MeshData* ZR_Embed_InverseCube();
extern "C" MeshData* ZR_Embed_Quad();
extern "C" MeshData* ZR_Embed_Spike();
extern "C" MeshData* ZR_Embed_Octahedron();

extern "C" BWImage ZR_Embed_Charset();

extern "C" i32 TexGen_BytesFor32BitImage(i32 width, i32 height);
extern "C" i32 TexGen_BytesForBWImage(i32 width, i32 height);
extern "C" void TexGen_SetRGBA(
	ColourU32* pixels, i32 width, i32 height, ColourU32 colour);
extern "C" void TexGen_FillRect(
	ColourU32* pixels, i32 texWidth, i32 texHeight, Point topLeft, Point size, ColourU32 colour);

extern "C" i32 TexGen_EncodeBW(
	u8* dest, const i32 destSize, ColourU32* pixels, const i32 w, const i32 h);
extern "C" i32 TexGen_DecodeBW(
	u8* source, const i32 sourceSize, ColourU32* target, const i32 w, const i32 h, ColourU32 solid, ColourU32 empty);

#endif // ZR_EMBEDDED_H
#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include "../ze_common/ze_common_full.h"

#define VW_ERROR_NONE 0
#define VW_ERROR_UNKNOWN 1
#define VW_ERROR_ALLOC_FAILED 2

typedef int VWError;

struct VWMeshData
{
	u32 numVerts;

	f32* verts;
	f32* uvs;
    f32* normals;    
};


struct VWBlock
{
	u8 type;
	u8 flags;
};

struct VWChunk
{
	i32 id;
	i32 size;
	i32 numBlocks;
	VWBlock* blocks;
	VWMeshData* mesh;
};

struct VWScene
{
	VWChunk* chunks;
	i32 numChunks;
};

extern "C"
VWError VW_AllocChunk(i32 size, VWChunk** result);
extern "C"
i32 VW_CalcIndex(Point3 chunkSize, Point3 pos);
extern "C"
void VW_Test();

#endif // VOXEL_WORLD_H
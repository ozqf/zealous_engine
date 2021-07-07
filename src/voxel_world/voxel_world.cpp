#include "../../headers/voxel_world.h"

#include <stdlib.h>

/**
 * size is per dimension, eg 8 == 8x8x8 cube
 */
extern "C"
VWError VW_AllocChunk(i32 size, VWChunk** result)
{
	ZE_ASSERT(size > 0, "Cannot make a VWChunk with zero size")
	ZE_ASSERT(result != NULL, "VW_AllocChunk must have a result pointer!")
	i32 totalBlocks = size * size * size;
	i32 sizeOfChunkStruct = sizeof(VWChunk);
	i32 sizeOfBlocksArray = totalBlocks * sizeof(VWBlock);
	u8* mem = (u8*)malloc(sizeOfChunkStruct + sizeOfBlocksArray);
	if (mem == NULL)
	{
		*result = NULL;
		return VW_ERROR_ALLOC_FAILED;
	}

	printf("%d bytes for chunk's %d blocks\n", sizeOfBlocksArray, totalBlocks);

	*result = (VWChunk*)mem;
	(*result)->size = size;
	(*result)->numBlocks = totalBlocks;
	(*result)->blocks = (VWBlock*)(mem + sizeOfChunkStruct);
	// zero out memory
	ZE_SET_ZERO((*result)->blocks, sizeOfBlocksArray)
	return VW_ERROR_NONE;
}

extern "C"
i32 VW_CalcIndex(Point3 chunkSize, Point3 pos)
{
	// check bounds
	if (pos.x < 0 || pos.x >= chunkSize.x) { return -1; }
	if (pos.y < 0 || pos.y >= chunkSize.y) { return -1; }
	if (pos.z < 0 || pos.z >= chunkSize.z) { return -1; }
	// Flat[x + HEIGHT* (y + WIDTH* z)] = Original[x, y, z], assuming Original[HEIGHT,WIDTH,DEPTH] 
	i32 i = pos.x + chunkSize.y * (pos.y + chunkSize.x * pos.z);
	return i;
}

internal i32 VW_IsSolid(VWBlock* blocks, Point3 size, Point3 pos)
{
	i32 i = VW_CalcIndex(size, pos);
	if (i == -1) { return NO; }
	return (blocks[i].type == VW_BLOCK_TYPE_EMPTY);
}

internal void VW_CalcBlockFlags(VWBlock* blocks, Point3 size, Point3 p)
{
	i32 i = VW_CalcIndex(size, p);
	if (i == -1) { return; }
	VWBlock* block = &blocks[i];
	// x
	if (VW_IsSolid(blocks, size, { p.x + 1, p.y, p.z }))
	{ block->flags |= VW_FACE_LEFT; }
	else { block->flags &= ~VW_FACE_LEFT; }
	if (VW_IsSolid(blocks, size, { p.x - 1, p.y, p.z }))
	{ block->flags |= VW_FACE_RIGHT; }
	else { block->flags &= ~VW_FACE_RIGHT; }
	// y
	if (VW_IsSolid(blocks, size, { p.x, p.y + 1, p.z }))
	{ block->flags |= VW_FACE_TOP; }
	else { block->flags &= ~VW_FACE_TOP; }
	if (VW_IsSolid(blocks, size, { p.x, p.y - 1, p.z }))
	{ block->flags |= VW_FACE_BOTTOM; }
	else { block->flags &= ~VW_FACE_BOTTOM; }
	// z
	if (VW_IsSolid(blocks, size, { p.x, p.y, p.z + 1 }))
	{ block->flags |= VW_FACE_FRONT; }
	else { block->flags &= ~VW_FACE_FRONT; }
	if (VW_IsSolid(blocks, size, { p.x, p.y, p.z - 1 }))
	{ block->flags |= VW_FACE_BACK; }
	else { block->flags &= ~VW_FACE_BACK; }
	
}

extern "C"
void VW_Test()
{
	printf("=== VOXEL WORLD TEST ===\n");
	printf("3D array access\n");
	Point3 size = { 3, 3, 3 };
	i32 total = size.x * size.y * size.z;
	for (i32 z = 0; z < size.z; ++z)
	{
		for (i32 y = 0; y < size.y; ++y)
		{
			for (i32 x = 0; x < size.x; ++x)
			{
				i32 i = VW_CalcIndex(size, { x, y, z });
				printf("Index for %d, %d, %d: %d\n",
					x, y, z, i);
			}
		}
	}
}

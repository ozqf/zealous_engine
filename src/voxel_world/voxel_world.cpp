#include "voxel_world.h"

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

	*result = (VWChunk*)mem;
	(*result)->size = size;
	(*result)->numBlocks = totalBlocks;
	(*result)->blocks = (VWBlock*)(mem + sizeOfChunkStruct);
	return VW_ERROR_NONE;
}

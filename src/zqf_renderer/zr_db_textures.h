#ifndef ZR_DB_TEXTURES_H
#define ZR_DB_TEXTURES_H

// Asset libraries:
#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"
#include "../../lib/openfbx/ofbx.h"

#include "zr_asset_db.h"

/**
 * Load an entire file, unaltered, into memory.
 * Must be freed by the caller after use
 */
extern "C" i32 ZRDB_StageRawFile(char* path, ZEByteBuffer* dest)
{
    FILE* f;
    i32 err = fopen_s(&f, path, "rb");
    if (err != 0)
    {
        printf("FAILED: Could not open file \"%s\" for reading\n", path);
        return ZE_ERROR_NOT_FOUND;
    }
    fseek(f, 0, SEEK_END);
    i32 size = ftell(f);
    fseek(f, 0, SEEK_SET);
    void* mem = malloc(size);
    if (mem == NULL)
    {
        return ZE_ERROR_ALLOCATION_FAILED;
    }
    *dest = Buf_FromMalloc(mem, size);
    fread((void*)dest->start, dest->capacity, 1, f);
    fclose(f);
    //printf("  staged \"%s\" (%d KB)\n", size / 1024);
    return ZE_ERROR_NONE;
}

static u8* ZRDB_LoadTextureToHeap(
    char* path, i32 bVerbose, int* x, int* y, i32 bFlipY)
{
    ZEByteBuffer b;
    ErrorCode err = ZRDB_StageRawFile(path, &b);
    if (err != ZE_ERROR_NONE)
    {
        return NULL;
    }
    
    // Load to heap:
    i32 comp;
    stbi_set_flip_vertically_on_load(bFlipY);
    u8* tex = stbi_load_from_memory(
        b.start, b.capacity, x, y, &comp, STBI_rgb_alpha);
    if (bVerbose == YES)
    { printf("Loaded img res %d, %d - comp %d\n", *x, *y, comp); }
    free(b.start);
    return tex;
}

static i32 ZRDB_LoadTexture(ZRAssetDB* assetDB, char* path, i32 bVerbose)
{
	ZRDB_CAST_TO_INTERNAL(assetDB, db)
	ZEByteBuffer buf;
	if (ZRDB_StageRawFile(path, &buf) != ZE_ERROR_NONE)
	{
		return 0;
	}
	u8* pixels;
	i32 x, y;
	u32 handle;
    pixels = ZRDB_LoadTextureToHeap(path, bVerbose, &x, &y, YES);
	db->uploader.UploadTexture(pixels, x, y, &handle);
    return 0;
}

#endif // ZR_DB_TEXTURES_H

#include "../../internal_headers/zengine_internal.h"

ze_internal i32 g_nextId = 1;

/////////////////////////////////////////////////////////////
// internal data types
/*struct ZEAssetHeader
{
    i32 id;
    i32 type;
    void* mem;
    i32 size;
};*/

/////////////////////////////////////////////////////////////
// functions
ze_external zErrorCode ZAssets_Init()
{
    return ZE_ERROR_NONE;
}

ze_external i32 ZAssets_GetTexIdByName(char* name)
{
    return 0;
}

ze_external ZRTexture *ZAssets_GetTexByName(char *name)
{
    return NULL;
}

ze_external ZRTexture *ZAssets_GetTexById(char *name)
{
    return NULL;
}

ze_external ZRTexture *ZAssets_AllocTex(i32 width, i32 height)
{
    i32 pixelBytes = (width * height) * sizeof(u32);
    i32 totalBytes = sizeof(ZRTexture) + pixelBytes;
    void* mem = Platform_Alloc(totalBytes);
    ZRTexture* tex = (ZRTexture*)mem;
    tex->data = (ColourU32 *)((u8 *)mem + sizeof(ZRTexture));

    tex->header.id = g_nextId++;
    tex->header.type = 1;
    tex->width = width;
    tex->height = height;
    return tex;
}

ze_external ZRMeshData* ZAssets_AllocMesh(i32 maxVerts)
{
    i32 numVertBytes = (maxVerts * 3) * sizeof(f32);
    i32 numUVBytes = (maxVerts * 2) * sizeof(f32);
    i32 numNormalBytes = (maxVerts * 3) * sizeof(f32);

    i32 totalBytes = sizeof(ZRMeshData) + numVertBytes + numUVBytes + numNormalBytes;
    u8* mem = (u8*)Platform_Alloc(totalBytes);
    ZRMeshData* result = (ZRMeshData*)mem;
    *result = {};
    result->numVerts = 0;
    result->maxVerts = maxVerts;
    
    result->verts = (f32*)(mem + sizeof(ZRMeshData));
    result->uvs = (f32*)(mem + sizeof(ZRMeshData) + numUVBytes);
    result->normals = (f32*)(mem + sizeof(ZRMeshData) + numUVBytes + numNormalBytes);
    return result;
}

ze_external zErrorCode ZAssets_LoadTex()
{
    return ZE_ERROR_NONE;
}

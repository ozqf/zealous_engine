
#include "../../internal_headers/zengine_internal.h"

// these are assets returned if assets requested are not found
// ze_internal ZRTexture* g_fallbackTexture = NULL;
// ze_internal ZRMeshAsset* g_fallbackMesh = NULL;
// ze_internal ZRMaterial* g_fallbackMaterial = NULL;

ze_internal ZEHashTable* g_table;

/////////////////////////////////////////////////////////////
// internal data types
/*struct ZEAssetHeader
{
    i32 id;
    i32 type;
    void* mem;
    i32 size;
};*/

ze_external zErrorCode ZAssets_Init()
{
    g_table = ZE_HashTable_Create(Platform_Alloc, 2048, NULL);
    return ZE_ERROR_NONE;
}

ze_external void ZAssets_PrintAll()
{
    printf("=== Asset Heap ===\n");
    for (i32 i = 0; i < g_table->m_maxKeys; ++i)
    {
        ZEHashTableKey* key = &g_table->m_keys[i];
        if (key->id == 0) { continue; }
        ZRAsset* asset = (ZRAsset*)key->data.ptr;
        printf("KeyId %d, keyHash %d. AssetId %d - type %d - sentinel %X\n",
            key->id, key->idHash, asset->id, asset->type, asset->sentinel);
    }
}

/////////////////////////////////////////////////////////////
// Generic retrieval
/////////////////////////////////////////////////////////////

internal i32 ZAssets_GetAssetIdByName(char* name)
{
    return ZE_Hash_djb2((uChar *)name);
}

internal ZRAsset *ZAssets_FindAssetById(i32 id)
{
    ZRAsset* asset = (ZRAsset*)g_table->FindData(id);
    return asset;
}

internal ZRAsset *ZAssets_FindAssetByName(char* name)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    return (ZRAsset*)g_table->FindData(id);
}

/////////////////////////////////////////////////////////////
// Specific retrieval
/////////////////////////////////////////////////////////////

ze_external ZRTexture *GetFallbackTexture()
{
    return ZAssets_GetTexByName(FALLBACK_TEXTURE_NAME);
}

ze_external ZRMaterial *GetFallbackMaterial()
{
    return NULL;
}

ze_external ZRMeshAsset *GetFallbackMesh()
{
    return NULL;
}

ze_external ZRTexture *ZAssets_GetTexById(i32 id)
{
    ZRAsset* asset = (ZRAsset*)g_table->FindPointer(id);
    if (asset == NULL) { return GetFallbackTexture(); }
    if (asset->type != ZE_ASSET_TYPE_TEXTURE) { return GetFallbackTexture(); }
    return (ZRTexture*)asset;
}

ze_external ZRTexture *ZAssets_GetTexByName(char *name)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    return ZAssets_GetTexById(id);
}

ze_external ZRMaterial* ZAssets_GetMaterialByName(char* name)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    ZRAsset* asset = ZAssets_FindAssetById(id);
    if (asset == NULL) { return GetFallbackMaterial(); }
    if (asset->type != ZE_ASSET_TYPE_MATERIAL) { return GetFallbackMaterial(); }
    return (ZRMaterial*)asset;
}

/////////////////////////////////////////////////////////////
// Create new assets
/////////////////////////////////////////////////////////////

// Textures are assigned on allocation
/*ze_external void ZAssets_AssignTexture(ZRTexture *tex, char *name)
{
    if (tex == NULL) { return; }
    if (name == NULL) { return; }
    i32 len = ZStr_Len(name);
    if (len == 0) { return; }
    i32 id = ZE_Hash_djb2((uChar *)name);
    // check this key isn't already registered
    ZRTexture* other = ZAssets_GetTexByName(name);
    if (g_table->FindKeyIndex(id) != ZE_LT_INVALID_INDEX)
    {
        return;
    }
    tex->header.id = id;
    tex->header.bIsDirty = YES;
    ZEHashTableData d;
    d.ptr = tex;
    g_table->Insert(tex->header.id, d);
    // TODO - save the filename and add pointer to asset header!
    // tex->header.fileName = InternStringSomewhere(name);
}*/

ze_external ZRTexture *ZAssets_AllocTex(i32 width, i32 height, char* name)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    // Check this id isn't in use
    ZRAsset* asset = ZAssets_FindAssetById(id);
    if (asset != NULL)
    {
        printf("Asset Id %d from name %s already exists\n", id, name);
        return NULL;
    }

    // alloc a new texture and assign id
    i32 pixelBytes = (width * height) * sizeof(u32);
    i32 totalBytes = sizeof(ZRTexture) + pixelBytes;
    void* mem = Platform_Alloc(totalBytes);
    ZRTexture* tex = (ZRTexture*)mem;
    *tex = {};
    tex->data = (ColourU32 *)((u8 *)mem + sizeof(ZRTexture));

    tex->header.id = id;
    tex->header.type = ZE_ASSET_TYPE_TEXTURE;
    tex->header.bIsDirty = YES;
    tex->header.sentinel = ZE_SENTINEL;
    tex->width = width;
    tex->height = height;
    
    // add to table
    ZEHashTableData d;
    d.ptr = tex;
    g_table->Insert(tex->header.id, d);
    printf("Allocated texture %s with asset id %d\n", name, id);
    return tex;
}

ze_external ZRMaterial *ZAssets_AllocMaterial(char *name)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    // Check this id isn't in use
    ZRAsset *asset = ZAssets_FindAssetById(id);
    if (asset != NULL)
    {
        printf("Asset Id %d from name %s already exists\n", id, name);
        return NULL;
    }
    ZRMaterial *mat = (ZRMaterial *)Platform_Alloc(sizeof(ZRMaterial));
    mat->header.id = id;
    mat->header.bIsDirty = YES;
    mat->header.type = ZE_ASSET_TYPE_MATERIAL;
    ZEHashTableData d;
    d.ptr = mat;
    g_table->Insert(id, d);
    return mat;
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

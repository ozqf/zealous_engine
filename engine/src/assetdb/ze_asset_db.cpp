#include "../../internal_headers/zengine_internal.h"

// these are assets returned if assets requested are not found
// ze_internal ZRTexture* g_fallbackTexture = NULL;
// ze_internal ZRMeshAsset* g_fallbackMesh = NULL;
// ze_internal ZRMaterial* g_fallbackMaterial = NULL;

ze_internal ZEHashTable* g_table;

ze_internal char* GetAssetTypeLabel(i32 type)
{
    switch (type)
    {
        case ZE_ASSET_TYPE_TEXTURE:
        return "texture";
        case ZE_ASSET_TYPE_MESH:
        return "mesh";
        case ZE_ASSET_TYPE_MATERIAL:
        return "material";
        default:
        return "none";
    }
}

ze_external void ZAssets_PrintAll()
{
    printf("=== Asset Heap ===\n");
    zeSize totalBytes = 0;
    for (i32 i = 0; i < g_table->m_maxKeys; ++i)
    {
        ZEHashTableKey* key = &g_table->m_keys[i];
        if (key->id == 0) { continue; }
        ZRAsset* asset = (ZRAsset*)key->data.ptr;
        printf("AssetId %d - type %d (%s) - size %lldKB",
            asset->id, asset->type, GetAssetTypeLabel(asset->type), asset->totalSize / 1024);
        totalBytes += asset->totalSize;
        if (asset->sentinel != ZE_SENTINEL)
        {
            printf("Bad sentinel! (%X)", asset->sentinel);
        }
        printf("\n");
    }
    printf("Total allocated: %lldKB\n", totalBytes / 1024);
}

ZCMD_CALLBACK(Exec_Manifest)
{
    ZAssets_PrintAll();
}

ze_external zErrorCode ZAssets_Init()
{
    g_table = ZE_HashTable_Create(Platform_Alloc, 2048, NULL);
    ZCmdConsole_RegisterInternalCommand("manifest", "List asset db contents", Exec_Manifest);
    return ZE_ERROR_NONE;
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
    ZRAsset *asset = (ZRAsset *)g_table->FindPointer(id);
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
    Platform_Fatal("Hit texture fallback");
    return NULL;
    // ZRAsset *asset = ZAssets_FindAssetByName(FALLBACK_TEXTURE_NAME);
    // ZE_ASSERT(asset != NULL, "No fallback texture found");
    // return (ZRTexture*)asset;
}

ze_external ZRMaterial *GetFallbackMaterial()
{
    Platform_Fatal("Hit material fallback");
    return NULL;
    // return ZAssets_GetMaterialByName(FALLBACK_MATERIAL_NAME);
}

ze_external ZRMeshAsset *GetFallbackMesh()
{
    Platform_Fatal("Hit mesh fallback");
    return NULL;
    // ZRAsset *asset = ZAssets_FindAssetByName(ZE_EMBEDDED_CUBE_NAME);
    // ZE_ASSERT(asset != NULL, "No fallback mesh found");
    // return (ZRMeshAsset*)asset;
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

ze_external ZRMaterial* ZAssets_GetMaterialById(i32 id)
{
    ZRAsset* asset = ZAssets_FindAssetById(id);
    if (asset == NULL) { return GetFallbackMaterial(); }
    if (asset->type != ZE_ASSET_TYPE_MATERIAL) { return GetFallbackMaterial(); }
    return (ZRMaterial*)asset;
}

ze_external ZRMaterial* ZAssets_GetMaterialByName(char* name)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    return ZAssets_GetMaterialById(id);
}

ze_external ZRMeshAsset* ZAssets_GetMeshById(i32 id)
{
    ZRAsset* asset = ZAssets_FindAssetById(id);
    if (asset == NULL)
    {
        printf("Mesh %d not found\n", id);
        Platform_Fatal("Mesh not found");
    }
    if (asset == NULL) { return GetFallbackMesh(); }
    if (asset->type != ZE_ASSET_TYPE_MESH) { return GetFallbackMesh(); }
    return (ZRMeshAsset*)asset;
}

ze_external ZRMeshAsset* ZAssets_GetMeshByName(char* name)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    // printf("Get mesh %s - id %d\n", name, id);
    return ZAssets_GetMeshById(id);
}

/////////////////////////////////////////////////////////////
// Create new assets
/////////////////////////////////////////////////////////////

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
    tex->header.totalSize = totalBytes;
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

    zeSize totalBytes = sizeof(ZRMaterial);
    ZRMaterial *mat = (ZRMaterial *)Platform_Alloc(totalBytes);
    mat->header.id = id;
    mat->header.bIsDirty = YES;
    mat->header.type = ZE_ASSET_TYPE_MATERIAL;
    mat->header.totalSize = totalBytes;
    mat->header.sentinel = ZE_SENTINEL;
    ZEHashTableData d;
    d.ptr = mat;
    g_table->Insert(id, d);
    return mat;
}

ze_external ZRMaterial* ZAssets_BuildMaterial(
    char* name, char* diffuseName, char* emissionName)
{
    ZRMaterial* mat = ZAssets_AllocMaterial(name);
    if (mat == NULL) { return NULL; }
    ZRTexture* tex = ZAssets_GetTexByName(diffuseName);
    if (tex != NULL)
    {
        mat->diffuseTexId = tex->header.id;
    }
    return mat;
}

ze_external ZRMeshAsset* ZAssets_AllocEmptyMesh(char* name, i32 maxVerts)
{
    i32 id = ZAssets_GetAssetIdByName(name);
    // Check this id isn't in use
    ZRAsset *asset = ZAssets_FindAssetById(id);
    if (asset != NULL)
    {
        printf("Asset Id %d from name %s already exists\n", id, name);
        return NULL;
    }

    i32 numVertBytes = (maxVerts * 3) * sizeof(f32);
    i32 numUVBytes = (maxVerts * 2) * sizeof(f32);
    i32 numNormalBytes = (maxVerts * 3) * sizeof(f32);

    i32 totalBytes = sizeof(ZRMeshAsset) + numVertBytes + numUVBytes + numNormalBytes;
    u8* mem = (u8*)Platform_Alloc(totalBytes);
    ZRMeshAsset *mesh = (ZRMeshAsset *)mem;
    *mesh = {};
    // register
    mesh->header.id = id;
    mesh->header.bIsDirty = YES;
    mesh->header.type = ZE_ASSET_TYPE_MESH;
    mesh->header.totalSize = totalBytes;
    mesh->header.sentinel = ZE_SENTINEL;

    g_table->InsertPointer(id, mesh);
    
    // setup data
    mesh->data.numVerts = 0;
    mesh->data.maxVerts = maxVerts;
    mesh->data.verts = (f32 *)(mem + sizeof(ZRMeshAsset));
    mesh->data.uvs = (f32 *)(mem + sizeof(ZRMeshAsset) + numVertBytes);
    mesh->data.normals = (f32 *)(mem + sizeof(ZRMeshAsset) + numVertBytes + numUVBytes);

    printf("Allocated mesh %s (%d max verts) with assetId %d\n",
        name, mesh->data.maxVerts, id);
    return mesh;
}

ze_external zErrorCode ZAssets_LoadTex()
{
    return ZE_ERROR_NONE;
}

/////////////////////////////////////////////////////////////
// Export
/////////////////////////////////////////////////////////////
ze_external ZAssetManager ZAssets_RegisterFunctions()
{
    ZAssetManager exportedFunctions = {};
    exportedFunctions.AllocTexture = ZAssets_AllocTex;
    exportedFunctions.AllocEmptyMesh = ZAssets_AllocEmptyMesh;
    exportedFunctions.AllocMaterial = ZAssets_AllocMaterial;
    exportedFunctions.BuildMaterial = ZAssets_BuildMaterial;

    exportedFunctions.GetTexById = ZAssets_GetTexById;
    exportedFunctions.GetTexByName = ZAssets_GetTexByName;
    exportedFunctions.GetMeshById = ZAssets_GetMeshById;
    exportedFunctions.GetMeshByName = ZAssets_GetMeshByName;
    exportedFunctions.GetMaterialById = ZAssets_GetMaterialById;
    exportedFunctions.GetMaterialByName = ZAssets_GetMaterialByName;

    return exportedFunctions;
}

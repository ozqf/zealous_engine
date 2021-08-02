#ifndef ZRGL_UPLOAD_H
#define ZRGL_UPLOAD_H

#include "ze_opengl_internal.h"

internal ZEBlobStore g_store;

ze_external void ZRGL_UploaderInit()
{
    ZE_InitBlobStore(Platform_Alloc, &g_store, 1024, sizeof(ZRGLHandles), 0);
}

ze_external void ZRGL_PrintHandles()
{
    printf("=== GPU Handles ===\n");
    i32 len = g_store.m_array->m_numBlobs;
    for (i32 i = 0; i < len; ++i)
    {
        ZRGLHandles* handles = (ZRGLHandles*)g_store.GetByIndex(i);
        printf("%d: assetId %d, type %d, ", i, handles->assetId, handles->assetType);
        switch(handles->assetType)
        {
            case ZE_ASSET_TYPE_TEXTURE:
            printf("Texture handle %d\n", handles->data.textureHandle);
            break;
            default:
            printf("\n");
            break;
        }
    }
}

ze_external u32 ZRGL_GetTextureHandle(i32 assetId)
{
    ZRGLHandles* handles = (ZRGLHandles*)g_store.GetById(assetId);
    if (handles != NULL)
    {
        return handles->data.textureHandle;
    }
    // we have no handle for this asset - find and upload it
    printf("Texture %d handle not found, uploading\n", assetId);
    ZRTexture *tex = ZAssets_GetTexById(assetId);
    if (tex->header.id == assetId)
    {
        // we need to upload this mesh and add a handle entry
        ZRGLHandles *newRecord = (ZRGLHandles *)g_store.GetFreeSlot(assetId);
        *newRecord = {};
        newRecord->assetId = assetId;
        newRecord->assetType = ZE_ASSET_TYPE_TEXTURE;
        ZRGL_UploadTexture(
            (u8 *)tex->data, tex->width, tex->height, &newRecord->data.textureHandle);
        return newRecord->data.textureHandle;
    }
    else
    {
        // we've been provided with a fallback asset.
        // return the handle for that instead
        return ZRGL_GetTextureHandle(tex->header.id);
    }
}

ze_external void ZRGL_UploadTexture(u8 *pixels, i32 width, i32 height, u32 *handle)
{
    if (pixels == NULL)
    {
        printf("ERROR UploadTex - pixels are null\n");
        return;
    }
    if (width <= 0)
    {
        printf("ERROR UploadTex - width <= 0\n");
        return;
    }
    if (height <= 0)
    {
        printf("ERROR UploadTex - height <= 0\n");
        return;
    }
    // Upload to GPU
    if (*handle == 0)
    {
        glGenTextures(1, handle);
        CHECK_GL_ERR
    }

    GLuint texID = *handle;
    glBindTexture(GL_TEXTURE_2D, texID);
    CHECK_GL_ERR
    // TODO: Assuming images are always RGBA here
    // Make sure conversion of pixel encoding is correct.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    CHECK_GL_ERR
    glGenerateMipmap(GL_TEXTURE_2D);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    CHECK_GL_ERR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECK_GL_ERR
    // Clear binding
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERR
    // if (bVerbose == YES)
    // { printf("Uploaded %s to tex handle %d\n", path, handle); }

    //return handle;
}

//////////////////////////////////////////////////////
// CREATE VAO
// VBO data layout:
// attribs: 0	1		2		3...
// [ Vertices | UVs | Normals | space for instance data... ]
//////////////////////////////////////////////////////
// TODO: glDataType is ignored! Always assuming FLOAT
// Type should be either GL_FLOAT or GL_DOUBLE!
ze_external void ZRGL_UploadMesh(ZRMeshData *data, ZRMeshHandles *result, u32 flags)
{
    ZE_ASSERT(data->numVerts <= data->maxVerts, "Bad upload - num verts > max verts\n");
    u32 vaoHandle, vboHandle;
    //////////////////////////////////////////
    // Get handles
    if (result->vao == 0)
    {
        // generate handles
        glGenVertexArrays(1, &vaoHandle);
        CHECK_GL_ERR
        glGenBuffers(1, &vboHandle);
        CHECK_GL_ERR
    }
    else
    {
        vaoHandle = result->vao;
        vboHandle = result->vbo;
    }
    // bind vao/vbo
    glBindVertexArray(vaoHandle);
    CHECK_GL_ERR
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    CHECK_GL_ERR

    i32 vec3Size = sizeof(Vec3);
    i32 vec2Size = sizeof(Vec2);
#if 0
    if (glDataType == GL_FLOAT)
    {
        vec3Size = sizeof(f32) * 3;
        vec2Size = sizeof(f32) * 2;
    }
    else if (glDataType == GL_DOUBLE)
    {
        vec3Size = sizeof(f64) * 3;
        vec2Size = sizeof(f64) * 2;
    }
    else
    {
        ILLEGAL_CODE_PATH
    }
#endif
    //////////////////////////////////////////
    // Calculate size
    i32 numVertBytes = data->numVerts * vec3Size;
    i32 numUVBytes = vec2Size * data->numVerts;
    i32 numNormalBytes = numVertBytes;
    i32 staticBytes = numVertBytes + numUVBytes + numNormalBytes;

    // Add space for up to 100 model view matrices:
    i32 maxInstances = ZR_MAX_BATCH_SIZE;
    i32 instanceDataBytes = maxInstances * sizeof(M4x4);

    i32 totalBytes = staticBytes + instanceDataBytes;

    /////////////////////////////////////////
    // upload sub-buffers and configure pointers

    i32 vertDataAttrib = ZRGL_DATA_ATTRIB_VERTICES;
    i32 uvDataAttrib = ZRGL_DATA_ATTRIB_UVS;
    i32 normalDataAttrib = ZRGL_DATA_ATTRIB_NORMALS;

    GLenum vboUsage = GL_DYNAMIC_DRAW;
    /*GLenum vboUsage = GL_STATIC_DRAW;
    if (flags | ZR_PREFAB_FLAG_DYNAMIC_MESH)
    {
        vboUsage = GL_DYNAMIC_DRAW;
    }*/
    // Allocate buffer for all three arrays verts-normals-uvs
    // send null ptr for data, we're not uploading yet
    glBufferData(GL_ARRAY_BUFFER, totalBytes, NULL, vboUsage);
    // Upload sub-buffers
    i32 vertOffset = 0;
    i32 uvOffset = numVertBytes;
    i32 normalOffset = numVertBytes + numUVBytes;

    // BUFFER: - All Verts | All Normals | All Uvs -
    glBufferSubData(GL_ARRAY_BUFFER, vertOffset, numVertBytes, data->verts);
    glBufferSubData(GL_ARRAY_BUFFER, normalOffset, numNormalBytes, data->normals);
    glBufferSubData(GL_ARRAY_BUFFER, uvOffset, numUVBytes, data->uvs);

    // enable use of static data
    glEnableVertexAttribArray(vertDataAttrib);
    glEnableVertexAttribArray(uvDataAttrib);
    glEnableVertexAttribArray(normalDataAttrib);
    CHECK_GL_ERR
    GLenum glDataType = GL_FLOAT;
    // setup how to read the static data sections
    glVertexAttribPointer(vertDataAttrib, 3, glDataType, GL_FALSE, 0, 0);
    glVertexAttribPointer(uvDataAttrib, 2, glDataType, GL_FALSE, 0, (void *)uvOffset);
    glVertexAttribPointer(normalDataAttrib, 3, glDataType, GL_FALSE, 0, (void *)normalOffset);
    CHECK_GL_ERR

    // printf("  Uploaded %d verts (%d bytes) to VAO %d, VBO %d\n",
    //     data->numVerts, totalBytes, vaoHandle, vboHandle);

    //ZRMeshHandles result = {};
    *result = {};
    result->vao = vaoHandle;
    result->vbo = vboHandle;
    result->vertexCount = data->numVerts;
    result->instanceDataOffset = staticBytes;
    result->totalVBOBytes = totalBytes;
    result->maxInstances = maxInstances;
    //return result;
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#endif // ZRGL_UPLOAD_H
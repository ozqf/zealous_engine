#ifndef ZRGL_UPLOAD_H
#define ZRGL_UPLOAD_H

#include "zrgl_internal.h"

static void ZRGL_UploadTexture(u8* pixels, i32 width, i32 height, u32* handle)
{
    // Upload to GPU
    glGenTextures(1, handle);

    GLuint texID = *handle;
	glBindTexture(GL_TEXTURE_2D, texID);

    // TODO: Assuming images are always RGBA here
    // Make sure conversion of pixel encoding is correct. 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Clear binding
    glBindTexture(GL_TEXTURE_2D, 0);
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
static void ZRGL_UploadMesh(MeshData* data, ZRMeshHandles* result, u32 flags)
{
    //////////////////////////////////////////
    // Bind
    // > acquire vao handle and Bind to it
    u32 vaoHandle;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);
    CHECK_GL_ERR
    // bind vbo
    u32 vboHandle;
    glGenBuffers(1, &vboHandle);
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
    
    GLenum vboUsage = GL_STATIC_DRAW;
    if (flags | ZR_PREFAB_FLAG_DYNAMIC_MESH)
    {
        vboUsage = GL_DYNAMIC_DRAW;
    }
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
    glVertexAttribPointer(uvDataAttrib, 2, glDataType, GL_FALSE, 0, (void*)uvOffset);
    glVertexAttribPointer(normalDataAttrib, 3, glDataType, GL_FALSE, 0, (void*)normalOffset);
    CHECK_GL_ERR

    /*if (bVerbose == YES)
    { printf("  Uploaded %d verts (%d bytes) to VAO %d \n",
        numVerts, totalBytes, vaoHandle); }*/
    
    //ZRMeshHandles result = {};
    *result = {};
    result->vao = vaoHandle;
    result->vbo = vboHandle;
    result->vertexCount = data->numVerts;
	result->instanceDataOffset = staticBytes;
	result->totalVBOBytes = totalBytes;
    result->maxInstances = maxInstances;
    //return result;
}

#endif // ZRGL_UPLOAD_H
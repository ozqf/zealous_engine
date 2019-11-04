#ifndef ZRGL_PREFABS
#define ZRGL_PREFABS

#include "zrgl_internal.h"

// Asset libraries:
#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb_image.h"

#include "../../lib/openfbx/ofbx.h"

#define ZQF_R_DEFAULT_DIFFUSE_TEX "data/W33_5.bmp"

// Try something like this for temporary staging... ?
#if 0
struct StackMalloc
{
    void* mem = NULL;
    StackMalloc(i32 bytes) { mem = malloc(bytes); }
    // Stack frame exit will auto call:
    ~StackMalloc() { free(mem); }
};
#endif

/**
 * Load an entire file, unaltered, into memory.
 * Must be freed by the caller after use
 */
extern "C" i32 ZRGL_StageRawFile(char* path, ZEByteBuffer* dest)
{
    FILE* f;
    i32 err = fopen_s(&f, path, "rb");
    if (err != 0)
    {
        printf("Could not open file \"%s\" for reading\n", path);
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

// Returns opengl handle or 0 if failed
// Length == pixels wide and high, each pixel being 4 floats
// As 1D, can't be big enough...?
static ZRDataTexture ZRGL_CreateDataTexture2D(
    ZRPlatform* plat, i32 length)
{
    ZRDataTexture tex = {};
    tex.bIs1D = NO;
    tex.width = length;
    tex.height = length;
    // Calc bytes required
    tex.numBytes = sizeof(Vec4) * (tex.width * tex.height);
    tex.mem = (Vec4*)plat->Allocate(tex.numBytes);
    ZE_ASSERT(tex.mem != NULL, "Failed to allocate memory for texture\n");
    ZE_SET_ZERO((u8*)tex.mem, tex.numBytes);
    glGenTextures(1, &tex.handle);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    CHECK_GL_ERR
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
        tex.width, tex.height, 0, GL_RGBA, GL_FLOAT, tex.mem);
    CHECK_GL_ERR
    return tex;
}

static ZRDataTexture ZRGL_CreateDataTexture1D(
    ZRPlatform* plat, i32 length)
{
    ZRDataTexture tex = {};
    tex.bIs1D = YES;
    tex.width = length;
    tex.height = 1;
    // Calc bytes required
    tex.numBytes = sizeof(Vec4) * tex.width;
    tex.mem = (Vec4*)plat->Allocate(tex.numBytes);
    ZE_ASSERT(tex.mem != NULL, "Failed to allocate memory for texture\n");
    ZE_SET_ZERO((u8*)tex.mem, tex.numBytes);
    glGenTextures(1, &tex.handle);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_1D, tex.handle);
    CHECK_GL_ERR
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F,
        tex.width, 0, GL_RGBA, GL_FLOAT, tex.mem);
    CHECK_GL_ERR
    return tex;
}

static u8* ZRGL_LoadTextureToHeap(
    char* path, i32 bVerbose, int* x, int* y, i32 bFlipY)
{
    ZEByteBuffer b;
    ErrorCode err = ZRGL_StageRawFile(path, &b);
    if (err != ZE_ERROR_NONE)
    {
        return 0;
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

static u32 ZRGL_LoadCubeMap(
    char** paths, // Must have a length of 6!
    i32 bVerbose)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    CHECK_GL_ERR
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    CHECK_GL_ERR
    printf("Creating skybox %d\n", textureID);
    /*
    GL_TEXTURE_CUBE_MAP_POSITIVE_X 	Right
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	Left
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	Top
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	Bottom
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	Back
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	Front
    */
    //
    int width, height;//, nrChannels;
    unsigned char *data;
    for(GLuint i = 0; i < 6; i++)
    {
        data = ZRGL_LoadTextureToHeap(paths[i], NO, &width, &height, NO);
        //data = stbi_load(paths[i], &width, &height, &nrChannels, STBI_rgb_alpha);
        if (data != NULL)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            CHECK_GL_ERR
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            CHECK_GL_ERR
            //printf("Loaded side %d: %s (%d / %d)\n", i, paths[i], width, height);
        }
        else
        {
            printf("  Failed to tex %s\n", paths[i]);
        }
        //stbi_image_free(data);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    CHECK_GL_ERR
    return textureID;
}

// Returns opengl handle or 0 if failed
static u32 ZRGL_LoadTexture2D(char* path, i32 bVerbose)
{
    i32 x, y;
    u8* tex = ZRGL_LoadTextureToHeap(path, bVerbose, &x, &y, YES);
    // Upload to GPU
    u32 handle;
    glGenTextures(1, &handle);

    GLuint texID = handle;
	glBindTexture(GL_TEXTURE_2D, texID);

    // TODO: Assuming images are always RGBA here
    // Make sure conversion of pixel encoding is correct. 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex );
    glGenerateMipmap(GL_TEXTURE_2D);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Clear binding
    glBindTexture(GL_TEXTURE_2D, 0);
    if (bVerbose == YES)
    { printf("Uploaded %s to tex handle %d\n", path, handle); }

    return handle;
}

//////////////////////////////////////////////////////
// CREATE VAO
// VBO data layout:
// attribs: 0	1		2		3...
// [ Vertices | UVs | Normals | space for instance data... ]
//////////////////////////////////////////////////////
// TODO: glDataType is ignored! Always assuming FLOAT
// Type should be either GL_FLOAT or GL_DOUBLE!
static ZRMeshHandles ZRGL_CreateVAOf(
    i32 numVerts, Vec3* verts, Vec2* uvs, Vec3* normals, u32 flags, i32 bVerbose)
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
    i32 numVertBytes = numVerts * vec3Size;
    i32 numUVBytes = vec2Size * numVerts;
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
    glBufferSubData(GL_ARRAY_BUFFER, vertOffset, numVertBytes, verts);
    glBufferSubData(GL_ARRAY_BUFFER, normalOffset, numNormalBytes, normals);
    glBufferSubData(GL_ARRAY_BUFFER, uvOffset, numUVBytes, uvs);

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

    if (bVerbose == YES)
    { printf("  Uploaded %d verts (%d bytes) to VAO %d \n",
        numVerts, totalBytes, vaoHandle); }
    
    ZRMeshHandles result = {};
    result.vao = vaoHandle;
    result.vbo = vboHandle;
    result.vertexCount = numVerts;
	result.instanceDataOffset = staticBytes;
	result.totalVBOBytes = totalBytes;
    result.maxInstances = maxInstances;
    return result;
}

//////////////////////////////////////////////////////
// LOAD FBX FILE
// Extracts the first mesh from an fbx file, converts to 32bit
// and creates a VAO
//////////////////////////////////////////////////////

// https://github.com/nem0/LumixEngine/blob/411eabc57a84b6607a2669f7c4cdba42e24807ce/src/renderer/editor/fbx_importer.cpp
static void ZRGL_ExamineFBXFile(ofbx::IScene* fbx)
{
    char* fileName = "./fbx_dump.txt";
    FILE* f;
    fopen_s(&f, fileName, "w");
    if (f == NULL)
    {
        printf("Failed to open file \"%s\" to dump FBX file\n", fileName);
        return;
    }
    fprintf(f, "--- Examine FBX file ---\n");
    const ofbx::GlobalSettings* settings = fbx->getGlobalSettings();
    const ofbx::UpVector up = settings->UpAxis;
    
    fprintf(f, "Up vector: %d\n", up);
    i32 numMeshes = fbx->getMeshCount();
    fprintf(f, "%d Meshes:\n", numMeshes);
    for (i32 i = 0; i < numMeshes; ++i)
    {
        ofbx::Mesh* m = (ofbx::Mesh*)fbx->getMesh(i);
        ofbx::Geometry* geo = (ofbx::Geometry*)m->getGeometry();
        fprintf(f, "%d: \"%s\". %d verts\n", i, m->name, geo->getVertexCount());
        
        const ofbx::Skin* skin = geo->getSkin();
        
        // Skin clusters
        if (skin != NULL)
        {
            fprintf(f, "Got skin with %d clusters\n", skin->getClusterCount());
            for (i32 j = 0; j < skin->getClusterCount(); ++j)
            {
                const ofbx::Cluster* skinNode = skin->getCluster(j);
                fprintf(f, "Skin Node %d: \"%s\" type: %d indices count: %d\n",
                    j,
                    skinNode->name,
                    skinNode->getType(),
                    skinNode->getIndicesCount());
            }
        }
    }

    i32 numObjects = fbx->getAllObjectCount();
    fprintf(f, "%d objects:\n", numObjects);
    #if 0
    ofbx::Object* obj = (ofbx::Object*)fbx->getRoot();
    do
    {
        if (obj->isNode())
        {
            printf("Root is node\n");
        }
        obj = NULL;
    } while (obj != NULL);
    #endif
    #if 1
    i32 numAnimations = fbx->getAnimationStackCount();
    fprintf(f, "Animation stacks: %d:\n", numAnimations);
    for (i32 i = 0; i < numAnimations; ++i)
    {
        const ofbx::AnimationStack* animStack = fbx->getAnimationStack(i);
        fprintf(f, "Stack %d: \"%s\"\n", i, animStack->name);
        // Layers and curve nodes don't have a count
        // just have to iterate until the end is found
        i32 stack_i = 0;
        const ofbx::AnimationLayer* layer = animStack->getLayer(stack_i);
        while (layer != NULL)
        {
            // 'Cause I keep forgetting: ctrl + alt <tilde to type a ¦
            if (strcmp(layer->name, "Armature|Idle") == 0)
            {
                fprintf(f, "  Layer: \"%s\"\n", layer->name);
                i32 numNodes = 0;
                i32 numBones = 0;
                i32 curveNode_i = 0;
                const ofbx::AnimationCurveNode* curveNode = layer->getCurveNode(curveNode_i);
                while(curveNode != NULL)
                {
                    numNodes++;
                    // do stuff
                    const ofbx::Object* boneObj = curveNode->getBone();
                    if (boneObj != NULL)
                    {
                        numBones++;
                        fprintf(f, "Bone: %s\n", boneObj->name);
                    }
                    curveNode_i++;
                    curveNode = layer->getCurveNode(curveNode_i);
                }
                fprintf(f, "  %d curve nodes, %d bones\n",
                    numNodes, numBones);
            }
            
            stack_i++;
            layer = animStack->getLayer(stack_i);
        }
    }
    fclose(f);
    #endif
}

static ZRMeshHandles ZRGL_LoadFBX(
    char* path, i32* vertexCount, Vec3 reScale, i32 bSwapYZ, i32 bVerbose)
{
    ZE_ASSERT(vertexCount != NULL, "No pointer to pass out vertex count provided")
    if (reScale.x == 0 || reScale.y == 0 || reScale.z == 0)
    {
        reScale.x = 1;
        reScale.y = 1;
        reScale.z = 1;
    }
    //////////////////////////////////////////
    // Stage
    //////////////////////////////////////////
    if (bVerbose == YES) { printf("Loading FBX file: \"%s\"\n", path); }
    ZEByteBuffer b;
    ErrorCode err = ZRGL_StageRawFile(path, &b);
    if (err != ZE_ERROR_NONE)
    {
        printf("  Error %d reading \"%s\"\n", err, path);
        return {};
    }
    // Extract FBX
    ofbx::IScene* s = ofbx::load(b.start, b.capacity);
    ZE_ASSERT(s != NULL, "openfbx load failed");
    printf("FBX file %s up axis is %d\n", path, s->getGlobalSettings()->UpAxis);

    // unstage raw file
    free(b.start);

    // Extract mesh
    const i32 meshIndex = 0;
    const ofbx::Mesh* mesh = s->getMesh(meshIndex);
    i32 numVerts = mesh->getGeometry()->getVertexCount();
    *vertexCount = numVerts;

    //////////////////////////////////////////
    // Extract data
    // TODO: What to do about having to converting from double to float here...
    const ofbx::Geometry* geometry = mesh->getGeometry();

    const ofbx::Vec3* verts64 = geometry->getVertices();
    const ofbx::Vec2* uvs64 = geometry->getUVs(meshIndex);
    const ofbx::Vec3* normals64 = geometry->getNormals();

    // TODO: These pointers are lost after this load!
    Vec3* verts = (Vec3*)malloc(sizeof(Vec3) * numVerts);
    Vec2* uvs = (Vec2*)malloc(sizeof(Vec2) * numVerts);
    Vec3* normals = (Vec3*)malloc(sizeof(Vec3) * numVerts);

    // Copy out 64 bit vecs to 32 bit vecs
    Vec3* vertCursor = verts;
    Vec2* uvsCursor = uvs;
    Vec3* normalsCursor = normals;
    for (i32 i = 0; i < numVerts; ++i)
    {
        if (bSwapYZ == YES)
        {
            // swap y and z. z is -y otherwise surfaces are inside out!
            vertCursor->x = (f32)verts64->x * reScale.x;
            vertCursor->y = (f32)verts64->z * reScale.y;
            vertCursor->z = -(f32)verts64->y * reScale.z;

            normalsCursor->x = (f32)normals64->x;
            normalsCursor->y = (f32)normals64->z;
            normalsCursor->z = -(f32)normals64->y;
        }
        else
        {
            vertCursor->x = (f32)verts64->x * reScale.x;
            vertCursor->y = (f32)verts64->y * reScale.y;
            vertCursor->z = (f32)verts64->z * reScale.z;

            normalsCursor->x = (f32)normals64->x;
            normalsCursor->y = (f32)normals64->y;
            normalsCursor->z = (f32)normals64->z;
        }
        

        uvsCursor->x = (f32)uvs64->x;
        uvsCursor->y = (f32)uvs64->y;

        vertCursor++; uvsCursor++; normalsCursor++;
        verts64++; uvs64++; normals64++;
    }
    // done with this now...?
    if (bVerbose == YES)
    {
        ZRGL_ExamineFBXFile(s);
    }
    s->destroy();

    // Upload
    ZRMeshHandles handles = ZRGL_CreateVAOf(
        numVerts, (Vec3*)verts, (Vec2*)uvs, (Vec3*)normals, 0, bVerbose
    );
    return handles;
}

static ZRPrefab* ZRGL_GetPrefab(i32 index)
{
    return &g_prefabs[index];
}

static void ZRGL_LoadDefaultPrefabs(i32 bVerbose)
{
    printf("ZRGL - load default prefabs\n");
    char* paths[6];
    #if 0 // Star field
    paths[ZR_CUBEMAP_LOAD_INDEX_RIGHT] = "data/skybox/ame_starfield/starfield_ft.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_LEFT] = "data/skybox/ame_starfield/starfield_bk.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_TOP] = "data/skybox/ame_starfield/starfield_up.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BOTTOM] = "data/skybox/ame_starfield/starfield_dn.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BACK] = "data/skybox/ame_starfield/starfield_rt.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_FRONT] = "data/skybox/ame_starfield/starfield_lf.tga";
    #endif
    #if 0 // Star field + nebula
    paths[ZR_CUBEMAP_LOAD_INDEX_RIGHT] = "data/skybox/ame_nebula/purplenebula_ft.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_LEFT] = "data/skybox/ame_nebula/purplenebula_bk.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_TOP] = "data/skybox/ame_nebula/purplenebula_up.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BOTTOM] = "data/skybox/ame_nebula/purplenebula_dn.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BACK] = "data/skybox/ame_nebula/purplenebula_rt.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_FRONT] = "data/skybox/ame_nebula/purplenebula_lf.tga";
    #endif
    #if 1 // Clouds
    paths[ZR_CUBEMAP_LOAD_INDEX_RIGHT] = "data/skybox/envmap_miramar/miramar_ft.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_LEFT] = "data/skybox/envmap_miramar/miramar_bk.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_TOP] = "data/skybox/envmap_miramar/miramar_up.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BOTTOM] = "data/skybox/envmap_miramar/miramar_dn.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BACK] = "data/skybox/envmap_miramar/miramar_rt.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_FRONT] = "data/skybox/envmap_miramar/miramar_lf.tga";
    #endif
    #if 0 // deep rocky canyon
    paths[ZR_CUBEMAP_LOAD_INDEX_RIGHT] = "data/skybox/elbrus/elbrus_ft.jpg";
    paths[ZR_CUBEMAP_LOAD_INDEX_LEFT] = "data/skybox/elbrus/elbrus_bk.jpg";
    paths[ZR_CUBEMAP_LOAD_INDEX_TOP] = "data/skybox/elbrus/elbrus_up.jpg";
    paths[ZR_CUBEMAP_LOAD_INDEX_BOTTOM] = "data/skybox/elbrus/elbrus_dn.jpg";
    paths[ZR_CUBEMAP_LOAD_INDEX_BACK] = "data/skybox/elbrus/elbrus_rt.jpg";
    paths[ZR_CUBEMAP_LOAD_INDEX_FRONT] = "data/skybox/elbrus/elbrus_lf.jpg";
    #endif
    #if 0 // Above stormy planet
    paths[ZR_CUBEMAP_LOAD_INDEX_RIGHT] = "data/skybox/mp_mandaris/mandaris_ft.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_LEFT] = "data/skybox/mp_mandaris/mandaris_bk.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_TOP] = "data/skybox/mp_mandaris/mandaris_up.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BOTTOM] = "data/skybox/mp_mandaris/mandaris_dn.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BACK] = "data/skybox/mp_mandaris/mandaris_rt.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_FRONT] = "data/skybox/mp_mandaris/mandaris_lf.tga";
    #endif
    #if 0 // Weird computery space station...thing?
    paths[ZR_CUBEMAP_LOAD_INDEX_RIGHT] = "data/skybox/mp_mainframe/mainframe_ft.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_LEFT] = "data/skybox/mp_mainframe/mainframe_bk.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_TOP] = "data/skybox/mp_mainframe/mainframe_up.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BOTTOM] = "data/skybox/mp_mainframe/mainframe_dn.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_BACK] = "data/skybox/mp_mainframe/mainframe_rt.tga";
    paths[ZR_CUBEMAP_LOAD_INDEX_FRONT] = "data/skybox/mp_mainframe/mainframe_lf.tga";
    #endif
    #if 0
    paths[ZR_CUBEMAP_LOAD_INDEX_RIGHT] = "data/W33_5.bmp";
    paths[ZR_CUBEMAP_LOAD_INDEX_LEFT] = "data/W33_5.bmp";
    paths[ZR_CUBEMAP_LOAD_INDEX_TOP] = "data/W33_5.bmp";
    paths[ZR_CUBEMAP_LOAD_INDEX_BOTTOM] = "data/W33_5.bmp";
    paths[ZR_CUBEMAP_LOAD_INDEX_BACK] = "data/W33_5.bmp";
    paths[ZR_CUBEMAP_LOAD_INDEX_FRONT] = "data/W33_5.bmp";
    #endif

    g_cubemapHandle = ZRGL_LoadCubeMap(paths, YES);

    // Default VAOs
    MeshData* d;

    d = ZR_Embed_Cube();
    g_cubeVAO = ZRGL_CreateVAOf(
        d->numVerts, (Vec3*)d->verts, (Vec2*)d->uvs, (Vec3*)d->normals, 0, bVerbose);

    d = ZR_Embed_InverseCube();
    g_inverseCubeVAO = ZRGL_CreateVAOf(
        d->numVerts, (Vec3*)d->verts, (Vec2*)d->uvs, (Vec3*)d->normals, 0, bVerbose);

    d = ZR_Embed_Quad();
    g_quadVAO = ZRGL_CreateVAOf(
        d->numVerts, (Vec3*)d->verts, (Vec2*)d->uvs, (Vec3*)d->normals, 0, bVerbose);
    
    d = ZR_Embed_Spike();
    g_spikeVAO = ZRGL_CreateVAOf(
        d->numVerts, (Vec3*)d->verts, (Vec2*)d->uvs, (Vec3*)d->normals, 0, bVerbose);
    
    g_defaultDiffuseHandle = 
        ZRGL_LoadTexture2D(ZQF_R_DEFAULT_DIFFUSE_TEX, bVerbose);

    /*
    "data/HugeGun.fbx"
    "data/box.fbx"
	"data/box2.fbx"

    ZQF_R_DEFAULT_DIFFUSE_TEX
    "data/LowFlak-AlbedoM.tga"
    "data/TemplePillar_albedo.tga"
    "data/lightmap_test.png"
    "data/TemplePillar_albedo.tga"
    */
    if (bVerbose == YES)
    { printf("RENDERER - Loading Draw object assets\n"); }
    
    ZRPrefab* prefab;
    // Cube
    prefab = &g_prefabs[ZR_PREFAB_TYPE_CUBE];
    prefab->geometry = g_cubeVAO;
    prefab->textures.diffuse = g_defaultDiffuseHandle;
    prefab->program = ZR_SHADER_TYPE_FALLBACK;

    // Inverse Cube
    prefab = &g_prefabs[ZR_PREFAB_TYPE_INVERSE_CUBE];
    prefab->geometry = g_inverseCubeVAO;
    prefab->textures.diffuse = g_defaultDiffuseHandle;
    prefab->program = ZR_SHADER_TYPE_FALLBACK;
    
    // Wall...?
    prefab = &g_prefabs[ZR_PREFAB_TYPE_WALL];
    prefab->geometry = g_cubeVAO;
    prefab->textures.diffuse = g_defaultDiffuseHandle;
    prefab->program = ZR_SHADER_TYPE_TEST;

    // Gun
    prefab = &g_prefabs[ZR_PREFAB_TYPE_GUN];
    prefab->geometry = ZRGL_LoadFBX(
        "data/HugeGun.fbx", &prefab->geometry.vertexCount, { 0.0008f, 0.0008f, 0.0008f}, NO, bVerbose);
    prefab->textures.diffuse = ZRGL_LoadTexture2D("data/LowFlak-AlbedoM.tga", bVerbose);
    prefab->program = ZR_SHADER_TYPE_TEST;
    // TODO: Scale here is ignored. scale is applied at load time (see above!)
    //prefab->scale = { 0.00001f, 0.00001f, 0.00001f };

    // Pillar
    prefab = &g_prefabs[ZR_PREFAB_TYPE_PILLAR];
    prefab->geometry = ZRGL_LoadFBX(
        "data/Pillar.fbx", &prefab->geometry.vertexCount, { 0.05f, 0.05f, 0.05f }, NO, bVerbose);
    prefab->textures.diffuse = ZRGL_LoadTexture2D("data/TemplePillar_albedo.tga", bVerbose);
    prefab->program = ZR_SHADER_TYPE_TEST;
    
    // Cube + spike for orientation tests
    prefab = &g_prefabs[ZR_PREFAB_TYPE_ORIENTATION_TEST];
    prefab->geometry = ZRGL_LoadFBX(
        "data/Box2.fbx", &prefab->geometry.vertexCount, {}, NO, bVerbose);
    prefab->textures.diffuse = ZRGL_LoadTexture2D(ZQF_R_DEFAULT_DIFFUSE_TEX, bVerbose);
    prefab->program = ZR_SHADER_TYPE_TEST;

    prefab = &g_prefabs[ZR_PREFAB_TYPE_BLOCK_COLOURED];
    d = ZR_Embed_Cube();
    // Creating a copied VAO here, could just use the basic cube
    prefab->geometry = ZRGL_CreateVAOf(
        d->numVerts, (Vec3*)d->verts, (Vec2*)d->uvs, (Vec3*)d->normals, 0, bVerbose);
    prefab->geometry.vertexCount = d->numVerts;
    // Is this necessary given that the prog doesn't use textures?
    //prefab->textures.diffuse = ZRGL_LoadTexture2D(ZQF_R_DEFAULT_DIFFUSE_TEX);
    prefab->program = ZR_SHADER_TYPE_BLOCK_COLOUR;

    prefab = &g_prefabs[ZR_PREFAB_TYPE_QUAD];
    // Billboard should use this quad mesh, but for debugging, draw a cube as it
    // doesn't potentially become invisible when rotations mess up!
    #if 1
    d = ZR_Embed_Quad();
    prefab->geometry = ZRGL_CreateVAOf(
        d->numVerts, (Vec3*)d->verts, (Vec2*)d->uvs, (Vec3*)d->normals, 0, bVerbose);
    prefab->geometry.vertexCount = g_quadVAO.vertexCount;
    #endif

    prefab = &g_prefabs[ZR_PREFAB_TYPE_QUAD_DYNAMIC];
    // VBO Should be dynamic!
    // Note: Or not... just render one at a time for now!
    prefab->geometry = g_quadVAO;
    prefab->textures.diffuse = ZRGL_LoadTexture2D("data/charset.bmp", bVerbose);
    prefab->geometry.vertexCount = g_quadVAO.vertexCount;
    prefab->program = ZR_SHADER_TYPE_TEXT;

    // Projectile Spike
    prefab = &g_prefabs[ZR_PREFAB_TYPE_SPIKE];
    prefab->geometry = g_spikeVAO;
    prefab->textures.diffuse = g_defaultDiffuseHandle;
    prefab->geometry.vertexCount = g_spikeVAO.vertexCount;
    
    // Character animation test
    prefab = &g_prefabs[ZR_PREFAB_TYPE_MAGE_TEST];
    Vec3 importScale = { 0.2f, 0.2f, 0.2f };
    //Vec3 importScale = { 0.05f, 0.05f, 0.05f };
    prefab->geometry = ZRGL_LoadFBX(
        "data/mage.fbx", &prefab->geometry.vertexCount, importScale, YES, YES);
    prefab->textures.diffuse = ZRGL_LoadTexture2D("data/Mage.png", bVerbose);
    prefab->program = ZR_SHADER_TYPE_TEST;
    // TODO: Scale here is ignored. scale is applied at load time (see above!)
    //prefab->scale = { 0.00001f, 0.00001f, 0.00001f };

}

#endif // ZRGL_PREFABS
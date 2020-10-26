#ifndef ZR_DB_FBX_H
#define ZR_DB_FBX_H

#include "zr_db_internal.h"
#include "../../lib/openfbx/ofbx.h"

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
#if 1
static i32 ZRGL_LoadFBX(
    ZEFileIO* files,
    char* path,
    Vec3 reScale,
    i32 bSwapYZ,
    i32 bVerbose,
	MeshData* meshData)
{
    ZE_ASSERT(meshData != NULL, "No meshData result provided")
    if (reScale.x == 0 || reScale.y == 0 || reScale.z == 0)
    {
        printf("Clearing FBX rescale\n");
        reScale.x = 1;
        reScale.y = 1;
        reScale.z = 1;
    }
    //////////////////////////////////////////
    // Stage
    //////////////////////////////////////////
    if (bVerbose == YES) { printf("Loading FBX file: \"%s\"\n", path); }
    ZEBuffer b;
    ErrorCode err = g_files.StageFile(path, NO, &b);
    if (err != ZE_ERROR_NONE)
    {
        printf("  Error %d reading \"%s\"\n", err, path);
        return ZE_ERROR_NOT_FOUND;
    }
    // Extract FBX
    ofbx::IScene* s = ofbx::load(b.start, b.capacity);
    ZE_ASSERT(s != NULL, "openfbx load failed");
    printf("FBX file %s up axis is %d\n", path, s->getGlobalSettings()->UpAxis);

    // unstage raw file
    g_files.FreeStagedFile(b.start);
    
    // Extract mesh
    const i32 meshIndex = 0;
    const ofbx::Mesh* mesh = s->getMesh(meshIndex);
    i32 numVerts = mesh->getGeometry()->getVertexCount();

    //////////////////////////////////////////
    // Extract data
    // TODO: What to do about having to converting from double to float here...
    const ofbx::Geometry* geometry = mesh->getGeometry();

    const ofbx::Vec3* verts64 = geometry->getVertices();
    const ofbx::Vec2* uvs64 = geometry->getUVs(meshIndex);
    const ofbx::Vec3* normals64 = geometry->getNormals();

    // TODO: These pointers are lost after this load!
    Vec3* verts = (Vec3*)g_alloc.Allocate(sizeof(Vec3) * numVerts);
    Vec2* uvs = (Vec2*)g_alloc.Allocate(sizeof(Vec2) * numVerts);
    Vec3* normals = (Vec3*)g_alloc.Allocate(sizeof(Vec3) * numVerts);

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
    *meshData = {};
    meshData->numVerts = numVerts;
    meshData->maxVerts = numVerts;
    meshData->verts = (f32*)verts;
    meshData->uvs = (f32*)uvs;
    meshData->normals = (f32*)normals;
    return 0;
}
#endif
#endif // ZR_DB_FBX_H
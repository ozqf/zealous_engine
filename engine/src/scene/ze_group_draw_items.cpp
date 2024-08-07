#include "ze_scene_internal.h"

#define MAX_DRAW_GROUPS 1024

ze_internal ZEBuffer g_scratch;

struct ZEDrawGroup
{
    i32 objType;
    u32 objHash;
    i32 numIndices;  
};

inline void CheckBufferCapacity(ZEBuffer* buf, zeSize additionalBytes)
{
    if (buf->Space() >= additionalBytes) { return; }
    // expand buffer
    zeSize newSize = buf->capacity * 2;
    printf("!Expanding draw cmd buf from %lldKB to %lldKB\n",
        buf->capacity / 1024, newSize / 1024);
    zeSize offset = buf->cursor - buf->start;
    buf->start = (i8*)Platform_Realloc(buf->start, newSize * 2);
    buf->cursor = buf->start + offset;
    buf->capacity = newSize;
}

inline void WriteTextCommand(ZEBuffer *buf, ZRDrawObj *textObj)
{
    // calculate required space
    char *str = textObj->data.text.text;
    i32 len = ZStr_Len(str) - 1; // ignore terminator at the end
    zeSize totalBytes = sizeof(ZR_DRAW_CMD_SPRITE_BATCH) + (len * sizeof(ZRQuad));
    CheckBufferCapacity(buf, totalBytes);
    // start a batch
    BUF_BLOCK_BEGIN_STRUCT(
        spriteBatch, ZRDrawCmdSpriteBatch, buf, ZR_DRAW_CMD_SPRITE_BATCH);
    if (textObj->data.text.charTextureId != 0)
    {
        spriteBatch->textureId = textObj->data.text.charTextureId;
    }
    else
    {
        spriteBatch->textureId = ZAssets_GetTexByName(
            FALLBACK_CHARSET_TEXTURE_NAME)->header.id;
    }
    spriteBatch->items = (ZRSpriteBatchItem *)buf->cursor;
    Vec2 charSize = Vec2_FromVec3(textObj->t.scale);
    f32 step = charSize.x * 2.f;

    // measure whole string for alignment
    // debugging - force alignment
    i32 alignment = textObj->data.text.alignment;
    // alignment = ZR_ALIGNMENT_CENTRE;
    i32 width = 0;
    i32 height = 0;
    ZStr_Measure2D(str, &width, &height);

    Vec3 origin = textObj->t.pos;
    switch (alignment)
    {
        case ZR_ALIGNMENT_CENTRE:
        {
            i32 firstLineLen = ZStr_MeasureLine(str);
            //printf("First line len %d\n", firstLineLen);
            // quad renders at centre point
            origin.x += charSize.x;
            origin.y -= charSize.x;
            origin.x -= step * (firstLineLen / 2);
            origin.y += step * (height / 2);
        }
        break;
        default: // NW
        origin.x += charSize.x;
        origin.y -= charSize.x;
        break;
    }
    
    Vec3 drawPos = origin;
    for (i32 i = 0; i < len; ++i)
    {
        char c = str[i];
        if (c == '\n')
        {
            switch (alignment)
            {
                case ZR_ALIGNMENT_CENTRE:
                {
                    i32 lineLen = ZStr_MeasureLine(&str[i + 1]);
                    //printf("Line len from %d (%c): %d\n", i, str[i + 1], lineLen);
                    drawPos.x = textObj->t.pos.x;
                    drawPos.x -= step * (lineLen / 2);
                    drawPos.y -= step;
                }
                break;
                default: // NW
                drawPos.x = origin.x;
                drawPos.y -= step;
                break;
            }
            continue;
        }
        Vec2 uvMin, uvMax;
        ZEAsciToCharsheetUVs(c, &uvMin, &uvMax);
        spriteBatch->AddItem(
            drawPos, charSize, uvMin, uvMax, 0, ColourF32FromU32(textObj->data.text.colour));
        drawPos.x += step;
    }

    // complete batch command
    spriteBatch->Finish(buf);
}

inline void WriteMeshCommand(ZEBuffer *buf, ZRDrawObj *obj)
{
    BUF_BLOCK_BEGIN_STRUCT(meshCmd, ZRDrawCmdMesh, buf, ZR_DRAW_CMD_MESH)
    meshCmd->obj = *obj;
}

inline void WriteBoundBoxCommand(ZEBuffer *buf, ZRDrawObj *obj)
{
    BUF_BLOCK_BEGIN_STRUCT(lines, ZRDrawCmdDebugLines, buf, ZR_DRAW_CMD_DEBUG_LINES);
    lines->verts = (ZRLineVertex*)buf->cursor;
    lines->bChained = NO;
    // allocate space for vertices and finished header
    lines->numVerts = 24;
    buf->cursor += (lines->numVerts * sizeof(ZRLineVertex));
    lines->header.size = buf->cursor - (i8 *)lines;
    // move aabb from object local to world space
    AABB local = obj->data.box.aabb;
    AABB aabb = AABB_LocalToWorld(
        obj->t.pos,
        local
    );

    // y
    lines->verts[0].pos = {aabb.min.x, aabb.min.y, aabb.min.z};
    lines->verts[1].pos = {aabb.min.x, aabb.max.y, aabb.min.z};

    lines->verts[2].pos = {aabb.max.x, aabb.min.y, aabb.min.z};
    lines->verts[3].pos = {aabb.max.x, aabb.max.y, aabb.min.z};

    lines->verts[4].pos = {aabb.min.x, aabb.min.y, aabb.max.z};
    lines->verts[5].pos = {aabb.min.x, aabb.max.y, aabb.max.z};

    lines->verts[6].pos = {aabb.max.x, aabb.min.y, aabb.max.z};
    lines->verts[7].pos = {aabb.max.x, aabb.max.y, aabb.max.z};

    // x
    lines->verts[8].pos = {aabb.min.x, aabb.min.y, aabb.min.z};
    lines->verts[9].pos = {aabb.max.x, aabb.min.y, aabb.min.z};

    lines->verts[10].pos = {aabb.min.x, aabb.max.y, aabb.min.z};
    lines->verts[11].pos = {aabb.max.x, aabb.max.y, aabb.min.z};

    lines->verts[12].pos = {aabb.min.x, aabb.min.y, aabb.max.z};
    lines->verts[13].pos = {aabb.max.x, aabb.min.y, aabb.max.z};

    lines->verts[14].pos = {aabb.min.x, aabb.max.y, aabb.max.z};
    lines->verts[15].pos = {aabb.max.x, aabb.max.y, aabb.max.z};

    // z
    lines->verts[16].pos = {aabb.min.x, aabb.min.y, aabb.min.z};
    lines->verts[17].pos = {aabb.min.x, aabb.min.y, aabb.max.z};

    lines->verts[18].pos = {aabb.min.x, aabb.max.y, aabb.min.z};
    lines->verts[19].pos = {aabb.min.x, aabb.max.y, aabb.max.z};

    lines->verts[20].pos = {aabb.max.x, aabb.min.y, aabb.min.z};
    lines->verts[21].pos = {aabb.max.x, aabb.min.y, aabb.max.z};

    lines->verts[22].pos = {aabb.max.x, aabb.max.y, aabb.min.z};
    lines->verts[23].pos = {aabb.max.x, aabb.max.y, aabb.max.z};

    /*
    lines->verts[2].pos = {aabb.min.x, aabb.max.y, aabb.min.z};
    lines->verts[3].pos = {aabb.max.x, aabb.max.y, aabb.min.z};
    
    lines->verts[4].pos = {aabb.min.x, aabb.max.y, aabb.min.z};
    lines->verts[5].pos = {aabb.min.x, aabb.max.y, aabb.max.z};

    lines->verts[6].pos = {aabb.max.x, aabb.min.y, aabb.max.z};
    lines->verts[7].pos = {aabb.max.x, aabb.max.y, aabb.max.z};
    */
    // printf("Wrote %d bytes of lines\n", lines->header.size);
}

inline void WriteLinesCommand(ZEBuffer *buf, ZRDrawObj *obj)
{
    BUF_BLOCK_BEGIN_STRUCT(lines, ZRDrawCmdDebugLines, buf, ZR_DRAW_CMD_DEBUG_LINES);
    lines->verts = (ZRLineVertex*)buf->cursor;
    lines->numVerts = obj->data.lines.numVerts;
    lines->bChained = obj->data.lines.bChained;
    buf->cursor += (lines->numVerts * sizeof(ZRLineVertex));
    lines->header.size = buf->cursor - (i8*)lines;
    for (i32 i = 0; i < lines->numVerts; ++i)
    {
        lines->verts[i] = obj->data.lines.verts[i];
    }
}

inline void AddTestLines(ZEBuffer *buf)
{
    BUF_BLOCK_BEGIN_STRUCT(lines, ZRDrawCmdDebugLines, buf, ZR_DRAW_CMD_DEBUG_LINES);
    // start vertices after struct
    lines->verts = (ZRLineVertex*)buf->cursor;
    lines->bChained = YES;
    // allocate space for vertices and finished header
    lines->numVerts = 4;
    buf->cursor += (lines->numVerts * sizeof(ZRLineVertex));
    lines->header.size = buf->cursor - (i8*)lines;
    // write vertices
    lines->verts[0].pos = { -4, -2, -3 };
    lines->verts[1].pos = { -1, -1, -1 };
    lines->verts[2].pos = { 1, 1, 1 };
    lines->verts[3].pos = { 2, 4, 4 };
    // printf("Wrote %d bytes of lines\n", lines->header.size);
}

inline ZEDrawGroup* FindGroupForObject(
    ZEDrawGroup* groups,
    i32* numGroups,
    i32 objType,
    u32 hash)
{
    ZEDrawGroup* result = NULL;
    for (i32 i = 0; i < *numGroups; ++i)
    {
        ZEDrawGroup* candidate = &groups[i];
        if (candidate->objHash == hash)
        {
            ZE_ASSERT(candidate->objType == objType, "Type mismatch in object group")
            result = candidate;
            break;
        }
    }
    if (result == NULL)
    {
        result = &groups[*numGroups];
        *result = {};
        *numGroups += 1;
        result->objHash = hash;
        result->objType = objType;
    }
    result->numIndices += 1;
    return result;
}

ze_internal void ZScene_BuildDrawGroups(ZEBuffer* scratch, ZRScene* scene)
{
    i32* numGroups = (i32*)scratch->cursor;
    *numGroups = 0;
    scratch->cursor += sizeof(i32);
    ZE_BUF_INIT_PTR_IN_PLACE(firstGroup, ZEDrawGroup, scratch)
    ZEDrawGroup* nextGroup = firstGroup;

    // first scan to find groups and their sizes so space can be allocated
    // second, with groups allocated, scan again and build commands
    i32 len = scene->objects.m_array->m_numBlobs;
    for (i32 i = 0; i < len; ++i)
    {
        ZRDrawObj *obj = (ZRDrawObj *)scene->objects.GetByIndex(i);
        switch (obj->data.type)
        {
            case ZR_DRAWOBJ_TYPE_MESH:
            FindGroupForObject(firstGroup, numGroups, obj->data.type, obj->CalcDataHash());
            break;

            case ZR_DRAWOBJ_TYPE_QUAD:
            FindGroupForObject(
                firstGroup, numGroups, obj->data.type, (u32)obj->data.quad.textureId);
            break;

            case ZR_DRAWOBJ_TYPE_LINES:
            break;

            case ZR_DRAWOBJ_TYPE_BOUNDING_BOX:
            break;

            case ZR_DRAWOBJ_TYPE_TEXT:
            break;
        }
    }
    #if 0 // frame log
    printf("--- Found %d draw groups ---\n", *numGroups);
    for (i32 i = 0; i < *numGroups; ++i)
    {
        ZEDrawGroup* group = &firstGroup[i];
        printf("%d - %d objects\n", group->objHash, group->numIndices);
    }
    #endif
}

ze_internal void SetupSceneProjection(ZRScene* scene, M4x4* target)
{
    ZE_ASSERT(scene != NULL, "Target scene is null")
    ZE_ASSERT(target != NULL, "Target matrix is null")
    f32 aspectRatio = Window_GetInfo().aspect;
    switch (scene->projectionMode)
    {
        case ZSCENE_PROJECTION_MODE_3D:
        ZE_SetupDefault3DProjection(target->cells, aspectRatio);
        break;

        case ZSCENE_PROJECTION_MODE_ORTHO:
        {
            f32 extent = scene->projectionInfo.presetOrth.verticalExtent;
            ZE_SetupOrthoProjection(target->cells, extent, aspectRatio);
        }
        break;

        default:
        *target = scene->projectionInfo.custom.projection;
        break;
    }
}

ze_internal void ZScene_WriteSceneWithGrouping(ZEBuffer *buf, ZRScene *scene)
{
    g_scratch.Clear(NO);
    i32 len = scene->objects.Count();
    // setup camera/projection
    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    setCamera->camera = scene->camera;
    SetupSceneProjection(scene, &setCamera->projection);
    //setCamera->projection = scene->projection;

    ZScene_BuildDrawGroups(&g_scratch, scene);
}


ze_internal void ZScene_WriteSceneNoGrouping(ZEBuffer *buf, ZRScene *scene)
{
    i32 len = scene->objects.m_array->m_numBlobs;
    // setup camera/projection
    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    setCamera->camera = scene->camera;
    SetupSceneProjection(scene, &setCamera->projection);

    for (i32 i = 0; i < len; ++i)
    {
        ZRDrawObj *obj = (ZRDrawObj *)scene->objects.GetByIndex(i);
        switch (obj->data.type)
        {
            case ZR_DRAWOBJ_TYPE_MESH:
            WriteMeshCommand(buf, obj);
            break;

            case ZR_DRAWOBJ_TYPE_QUAD:
            WriteSingleQuadCommand(buf, obj);
            break;

            case ZR_DRAWOBJ_TYPE_LINES:
            WriteLinesCommand(buf, obj);
            break;

            case ZR_DRAWOBJ_TYPE_BOUNDING_BOX:
            WriteBoundBoxCommand(buf, obj);
            break;

            case ZR_DRAWOBJ_TYPE_TEXT:
            WriteTextCommand(buf, obj);
            break;
        }
    }
    // clear depth for next scene
    BUF_BLOCK_BEGIN_STRUCT(clearCmd, ZRDrawCmdClearBuffer, buf, ZR_DRAW_CMD_CLEAR_BUFFER);
    
}


///////////////////////////////////////////////////////////
// External
///////////////////////////////////////////////////////////

ze_external void ZScene_WriteDrawCommands(ZEBuffer *buf, ZRScene *scene)
{
    ZScene_WriteSceneNoGrouping(buf, scene);
    //ZScene_WriteSceneWithGrouping(buf, scene);
}

ze_external void ZScene_InitGrouping()
{
    g_scratch = Buf_FromMalloc(Platform_Alloc, MegaBytes(1));
}

#include "ze_scene_internal.h"

internal void WriteTextCommand(ZEBuffer* buf, ZRDrawObj* textObj)
{
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

    char* str = textObj->data.text.text;
    i32 len = ZStr_Len(str) - 1; // ignore terminator at the end
    Vec2 charSize = Vec2_FromVec3(textObj->t.scale);
    f32 step = charSize.x * 2.f;
    Vec3 origin = textObj->t.pos;
    origin.x += charSize.x;
    origin.y -= charSize.x;
    Vec3 drawPos = origin;
    for (i32 i = 0; i < len; ++i)
    {
        char c = str[i];
        if (c == '\n')
        {
            drawPos.x = origin.x;
            drawPos.y -= step;
            continue;
        }
        Vec2 uvMin, uvMax;
        ZEAsciToCharsheetUVs(c, &uvMin, &uvMax);
        spriteBatch->AddItem(drawPos, charSize, uvMin, uvMax);
        drawPos.x += step;
    }

    // complete batch command
    spriteBatch->Finish(buf);
}

internal void WriteMeshCommand(ZEBuffer* buf, ZRDrawObj* obj)
{
    BUF_BLOCK_BEGIN_STRUCT(meshCmd, ZRDrawCmdMesh, buf, ZR_DRAW_CMD_MESH)
    meshCmd->obj = *obj;
}

internal void WriteBoundBoxCommand(ZEBuffer* buf, ZRDrawObj* obj)
{
    BUF_BLOCK_BEGIN_STRUCT(lines, ZRDrawCmdDebugLines, buf, ZR_DRAW_CMD_DEBUG_LINES);
    lines->verts = (ZRLineVertex*)buf->cursor;
    lines->bChained = NO;
    // allocate space for vertices and finished header
    lines->numVerts = 8;
    buf->cursor += (lines->numVerts * sizeof(ZRLineVertex));
    lines->header.size = buf->cursor - (i8 *)lines;
    AABB local = obj->data.box.aabb;
    AABB aabb = AABB_LocalToWorld(
        obj->t.pos,
        local
    );
    
    lines->verts[0].pos = {aabb.min.x, aabb.min.y, aabb.min.z};
    lines->verts[1].pos = {aabb.min.x, aabb.max.y, aabb.min.z};
    
    lines->verts[2].pos = {aabb.min.x, aabb.max.y, aabb.min.z};
    lines->verts[3].pos = {aabb.max.x, aabb.max.y, aabb.min.z};
    
    lines->verts[4].pos = {aabb.min.x, aabb.max.y, aabb.min.z};
    lines->verts[5].pos = {aabb.min.x, aabb.max.y, aabb.max.z};

    lines->verts[6].pos = {aabb.max.x, aabb.min.y, aabb.max.z};
    lines->verts[7].pos = {aabb.max.x, aabb.max.y, aabb.max.z};

    // printf("Wrote %d bytes of lines\n", lines->header.size);
}

internal void AddTestLines(ZEBuffer* buf)
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
    lines->verts[0].pos = { -2, -2, -2 };
    lines->verts[1].pos = { 2, 2, 2 };
    lines->verts[2].pos = { 2, -2, 2 };
    lines->verts[3].pos = { -2, 2, -2 };
    // printf("Wrote %d bytes of lines\n", lines->header.size);
}

ze_external void ZScene_WriteDrawCommands(ZEBuffer *buf, ZRScene *scene)
{
    i32 len = scene->objects.m_array->m_numBlobs;
    ZE_PRINTF("Write scene %d - %d objects\n",
              scene->id, len);

    // setup camera/projection
    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    setCamera->camera = scene->camera;
    setCamera->projection = scene->projection;

    // AddTestLines(buf);
    
    for (i32 i = 0; i < len; ++i)
    {
        ZRDrawObj *obj = (ZRDrawObj *)scene->objects.GetByIndex(i);
        switch (obj->data.type)
        {
            case ZR_DRAWOBJ_TYPE_MESH:
            WriteMeshCommand(buf, obj);
            break;

            case ZR_DRAWOBJ_TYPE_BOUNDING_BOX:
            WriteBoundBoxCommand(buf, obj);
            break;

            case ZR_DRAWOBJ_TYPE_TEXT:
            WriteTextCommand(buf, obj);
            break;
        }
    }
    BUF_BLOCK_BEGIN_STRUCT(meshCmd, ZRDrawCmdDebugLines, buf, ZR_DRAW_CMD_DEBUG_LINES)
    
}

ze_external void ZScene_InitGrouping()
{

}

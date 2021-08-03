#include "ze_scene_internal.h"



ze_external void ZScene_WriteDrawCommands(ZEBuffer *buf, ZRScene *scene)
{
    i32 len = scene->objects.m_array->m_numBlobs;
    ZE_PRINTF("Write scene %d - %d objects\n",
              scene->id, len);

    // setup camera/projection
    BUF_BLOCK_BEGIN_STRUCT(setCamera, ZRDrawCmdSetCamera, buf, ZR_DRAW_CMD_SET_CAMERA);
    setCamera->camera = scene->camera;
    setCamera->projection = scene->projection;
    
    // start a batch
    BUF_BLOCK_BEGIN_STRUCT(spriteBatch, ZRDrawCmdSpriteBatch, buf, ZR_DRAW_CMD_SPRITE_BATCH);
    spriteBatch->textureId = ZAssets_GetTexByName(FALLBACK_TEXTURE_NAME)->header.id;
    spriteBatch->textureId = ZAssets_GetTexByName(FALLBACK_CHARSET_TEXTURE_NAME)->header.id;
    spriteBatch->items = (ZRSpriteBatchItem *)buf->cursor;

    char chars[] = {'A', 'B', 'C'};
    // iterate objects and add to batch
    for (i32 i = 0; i < len; ++i)
    {
        ZRDrawObj *obj = (ZRDrawObj *)scene->objects.GetByIndex(i);
        Vec3 p = obj->t.pos;
        // spriteBatch->AddItem(p, {0.25, 0.25}, {0.25, 0.25}, {0.25, 0.25});
        Vec2 uvMin, uvMax;
        ZEAsciToCharsheetUVs(chars[i % 3], &uvMin, &uvMax);
        spriteBatch->AddItem(p, {0.25, 0.25}, uvMin, uvMax);
    }

    // complete batch command
    spriteBatch->Finish(buf);
}

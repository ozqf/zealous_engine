#ifndef ZE_DRAW_COMMANDS_H
#define ZE_DRAW_COMMANDS_H

#include "../zengine.h"


inline void WriteSingleQuadCommand(ZEBuffer *buf, ZRDrawObj *quadObj)
{
    zeSize totalBytes = sizeof(ZRDrawCmdSpriteBatch) + sizeof(ZRQuad);
    if (buf->Space() < totalBytes)
    {
        printf("Draw cmd buf full\n");
        return;
    }
    //CheckBufferCapacity(buf, totalBytes);
    // start a batch
    BUF_BLOCK_BEGIN_STRUCT(
        spriteBatch, ZRDrawCmdSpriteBatch, buf, ZR_DRAW_CMD_SPRITE_BATCH);
    spriteBatch->textureId = quadObj->data.quad.textureId;
    spriteBatch->items = (ZRSpriteBatchItem *)buf->cursor;
    ZRQuad *quad = &quadObj->data.quad;
    // printf("Draw single quad, tex Id %d uvs %.3f, %.3f to %.3f, %.3f\n",
    //     quad->textureId, quad->uvMin.x, quad->uvMin.y, quad->uvMax.x, quad->uvMax.y);
    f32 radians = M3x3_GetEulerAnglesRadians(quadObj->t.rotation.cells).z;
    i32 len = 1;
    spriteBatch->AddItem(
        quadObj->t.pos,
        { quadObj->t.scale.x, quadObj->t.scale.y },
        quad->uvMin,
        quad->uvMax,
        radians,
        quad->colour);
    
    // complete batch command
    spriteBatch->Finish(buf);
}


#endif // ZE_DRAW_COMMANDS_H
#ifndef ZRGL_SPRITES_H
#define ZRGL_SPRITES_H

#include "zrgl_internal.h"

/*
* Create atlas
    * Create hashtable of sprites for each atlas (use pre-existing format?)
    * Sprite type: uv coords on spritesheet.
* Create temp VBOs for render calls
On render:
* Scan groups for sprites - grouped by atlas (texture really)
* Per group count num sprites, assign a VBO (and expand it if necessary)
* Setup VBO verts as quads for each sprite using that atlas
    (normals will always be z forward or backward)
* Draw
*/


// global sprite list, shared by all atlases
internal ZRSprite g_sprites[4];
// test atlas - contains reference to texture
internal ZRSpriteAtlas g_atlas;

internal i32 ZR_GetSpriteAtlasId(i32 id)
{
                                                                                                                                                                                                                            
}

internal void ZR_InitSprite(ZRSprite* spr, char* name, i32 atlasId, Vec2 uvMin, Vec2 uvMax)
{
    *spr = {};
    spr->id = ZE_Hash_djb2((u8*)name);
    spr->atlasId = atlasId;
    spr->uvMin = uvMin;
    spr->uvMax = uvMax;
    printf("Make sprite %d: \"%s\"\n", spr->id, name);
}

internal void ZR_AtlasInit()
{
    printf("ZR Atlas init\n");
    ZRAssetDB* db = AssetDb();

    g_atlas = {};
    g_atlas.id = 1;
    g_atlas.numSprites = 4;
    g_atlas.pixPerUnit = 16;
    g_atlas.textureHandle = db->GetTextureByName(db, ZRDB_TEX_NAME_SHEET_TEST)->apiHandle;
    ZR_InitSprite(&g_sprites[0], "sprite1", 1, { 0, 0.5f }, { 0.5f, 1 });
    ZR_InitSprite(&g_sprites[1], "sprite2", 1, { 0.5f, 1 }, { 0.5f, 1 });
}

#endif // ZRGL_SPRITES_H
#ifndef ZRGL_SPRITES_H
#define ZRGL_SPRITES_H

#include "zrgl_internal.h"

/*
* Create atlas
    * Create hashtable of sprites for each atlas (use pre-existing format?)
* Create temp VBOs for render calls
On render:
* Scan groups for sprites - grouped by atlas (texture really)
* Per group count num sprites, assign a VBO (and expand it if necessary)
* Setup VBO verts as quads for each sprite using that atlas
    (normals will always be z forward or backward)
* Draw
*/

#endif // ZRGL_SPRITES_H
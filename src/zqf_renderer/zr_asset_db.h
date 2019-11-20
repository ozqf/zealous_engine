#ifndef ZR_ASSET_DB_H
#define ZR_ASSET_DB_H

#include "../zqf_renderer.h"
/**
 * Ultra simple way to store assets and retrieve by name or opengl handle
 */
extern "C" i32 ZRDB_RegisterTexture(char* fileName, void* data, i32 dataSize, i32 apiHandle);
extern "C" i32 ZRDB_GetTexIndexByName(char* name);
extern "C" i32 ZRDB_GetTexHandleByIndex(i32 index);

#endif // ZR_ASSET_DB_H
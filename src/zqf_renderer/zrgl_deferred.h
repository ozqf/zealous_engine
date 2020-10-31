#ifndef ZRGL_DEFERRED_H
#define ZRGL_DEFERRED_H

#include "zrgl_internal.h"

extern "C" ErrorCode ZRGL_InitDeferred(i32 scrWidth, i32 scrHeight);

extern "C" ZRGroupingStats ZR_DrawSceneDeferred(
    ZRSceneFrame *sceneCmd, ZEBuffer *scratch, ScreenInfo scrInfo);
extern "C" void ZRGL_DrawGBufferDebugQuads(f32 aspectRatio);
extern "C" ZRGroupingStats ZR_DrawSceneDeferred(
    ZRSceneFrame *sceneCmd, ZEBuffer *scratch, ScreenInfo scrInfo);

#endif // ZRGL_DEFERRED_H

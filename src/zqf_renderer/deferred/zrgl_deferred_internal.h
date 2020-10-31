#ifndef ZRGL_DEFERRED_INTERNAL_H
#define ZRGL_DEFERRED_INTERNAL_H

#include "../zrgl_deferred.h"

static ZRGBuffer g_gBuffer;
static GLuint g_gBufTextureSampler;

static ZRShader g_shdrBuildGBuffer;
static ZRShader g_shdrCombineGBuffer;

static ZRShader g_shdrGBufferPointLight;
static ZRShader g_shdrGBufferDirectLight;
static ZRShader g_shdrGBufferVolumeLight;

#endif // ZRGL_DEFERRED_INTERNAL_H

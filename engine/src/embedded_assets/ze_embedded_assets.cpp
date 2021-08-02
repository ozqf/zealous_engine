/*
Loads embedded primitives and assets into the asset db.
*/
#include "../../internal_headers/zengine_internal.h"

#include "bw_charset.h"

#define FALLBACK_CHARSET_NAME "fallback_charset"

internal void CreateFallbackCharset()
{
	ZRTexture* tex = ZAssets_AllocTex(bw_charset_width, bw_charset_height, "fallback_charset");
	// ZAssets_AssignTexture(tex, FALLBACK_CHARSET_NAME);
	printf("Assigned %s to %d\n", FALLBACK_CHARSET_NAME, tex->header.id);
}

ze_external zErrorCode ZEmbedded_Init()
{
	ZRTexture* tex = ZAssets_AllocTex(64, 64, "fallback_texture");
	ZGen_FillTexture(tex, { 255, 0, 255, 255 });
	CreateFallbackCharset();
	return ZE_ERROR_NONE;
}

/*
Loads embedded primitives and assets into the asset db.
*/
#include "../../internal_headers/zengine_internal.h"

#include "bw_charset.h"

ze_external zErrorCode ZEmbedded_Init()
{
	ZRTexture *tex;
	// magenta fallback
	tex = ZAssets_AllocTex(64, 64, FALLBACK_TEXTURE_NAME);
	ZGen_FillTexture(tex, { 255, 0, 255, 255 });

	// embedded black and white charset
	tex = ZAssets_AllocTex(bw_charset_width, bw_charset_height, FALLBACK_CHARSET_TEXTURE_NAME);
	TexGen_DecodeBW(
		bw_charset_bytes,
		bw_charset_num_bytes,
		tex->data,
		bw_charset_width,
		bw_charset_height,
		COLOUR_U32_GREEN, COLOUR_U32_BLACK);

	// embedded black and white charset
	tex = ZAssets_AllocTex(
		bw_charset_width, bw_charset_height, FALLBACK_CHARSET_SEMI_TRANSPARENT_TEXTURE_NAME);
	TexGen_DecodeBW(
		bw_charset_bytes,
		bw_charset_num_bytes,
		tex->data,
		bw_charset_width,
		bw_charset_height,
		COLOUR_U32_GREEN, COLOUR_U32_RED);
	// printf("Assigned %s to %d\n", FALLBACK_CHARSET_TEXTURE_NAME, tex->header.id);

	return ZE_ERROR_NONE;
}

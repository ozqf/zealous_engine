/**
 * Generate embedded/procedural assets
 */

#include "zr_db_internal.h"
#include "../zr_embedded/zr_embedded.h"

///////////////////////////////////////////////////////////////////////////
// Load embedded assets
///////////////////////////////////////////////////////////////////////////

static void ZRDB_WriteBWHeaderFile(
	char* name, u8* bytes, i32 numBytes, i32 w, i32 h, i32 bytesPerRow)
{
	char* fileName = "./bw_file.h";
    FILE* f;
    fopen_s(&f, fileName, "w");
    if (f == NULL)
    {
        printf("Failed to open file \"%s\" black & white header\n", fileName);
        return;
    }
	fprintf(f, "#ifndef BLACK_AND_WHITE_%s\n", name);
	fprintf(f, "#define BLACK_AND_WHITE_%s\n", name);
	fprintf(f, "static char* bw_%s_name = \"%s\";\n", name, name);
	fprintf(f, "static i32 bw_%s_width = %d;\n", name, w);
	fprintf(f, "static i32 bw_%s_height = %d;\n", name, h);
	fprintf(f, "static i32 bw_%s_num_bytes = %d;\n", name, numBytes);
	fprintf(f, "static u8 bw_%s_bytes[] = {\n", name);
	i32 rowCount = 0;
	i32 count = 0;
	u8* end = bytes + numBytes;
	while(bytes < end)
	{
		fprintf(f, "%d", *bytes);
		rowCount++;
		count++;
		if (count < numBytes)
		{
			fprintf(f, ",");
		}
		if (!(count % bytesPerRow))
		{
			rowCount = 0;
			fprintf(f, "\n");
		}
		bytes++;
	}
	fprintf(f, "};\n");
    fprintf(f, "#endif // BLACK_AND_WHITE_%s\n", name);
	fclose(f);
}

static void ZRDB_GenBWTexture(ZRAssetDB* db, ZRDBTexture* tex)
{
	printf("=== ZRDB Tex to B&W ===\n");
	i32 w = tex->width, h = tex->height;
	printf("Converting \"%s\" (%d, %d)\n",
		tex->header.fileName, w, h);
	i32 bytes = TexGen_BytesForBWImage(w, h);
	printf("\t%d bytes\n", bytes);

	u8* bwPixels = (u8*)g_alloc.Allocate(bytes);
	i32 err = TexGen_EncodeBW(bwPixels, bytes, (ColourU32*)tex->data, w, h);
	if (err != ZE_ERROR_NONE)
	{
		printf("\tError generating BW img\n");
		g_alloc.Free(bwPixels);
		return;
	}

	ZRDB_WriteBWHeaderFile("charset", bwPixels, bytes, w, h, 16);
	
	////////////////////////
	// Decode
	ColourU32 white = { 255, 255, 255, 255 };
	ColourU32 black = { 0, 0, 0, 255 };
	ColourU32 green = { 0, 255, 0, 255 };
	i32 resultBytes = TexGen_BytesFor32BitImage(w, h);
	ColourU32* pixels = (ColourU32*)g_alloc.Allocate(resultBytes);
	TexGen_DecodeBW(bwPixels, bytes, pixels, w, h, green, black);

	ZRDB_RegisterTexture(db, "test", pixels, resultBytes, w, h, 0);
}

static ZRDBTexture* ZRDB_LoadBWImage(ZRAssetDB* handle, BWImage img)
{
	ColourU32 white = { 255, 255, 255, 255 };
	ColourU32 black = { 0, 0, 0, 255 };
	ColourU32 green = { 0, 255, 0, 255 };
	i32 resultBytes = TexGen_BytesFor32BitImage(img.w, img.h);
	ColourU32* pixels = (ColourU32*)g_alloc.Allocate(resultBytes);
	TexGen_DecodeBW(img.bytes, img.numBytes, pixels, img.w, img.h, white, black);

	i32 i = ZRDB_RegisterTexture(handle, img.name, pixels, resultBytes, img.w, img.h, 0);
	ZRDB_CAST_TO_INTERNAL(handle, db)
	return &db->textures[i];
}

static ZRDBTexture* ZRDB_GenSolidTexture(ZRAssetDB* handle, char* name, ColourU32 colour)
{
	const i32 w = 2, h = 2;
	i32 bytes = TexGen_BytesFor32BitImage(w, h);
	ColourU32* pixels = (ColourU32*)g_alloc.Allocate(bytes);
	TexGen_SetRGBA(pixels, w, h, colour);
	i32 i = ZRDB_RegisterTexture(handle, name, pixels, bytes, w, h, 0);
	ZRDB_CAST_TO_INTERNAL(handle, db)
	return &db->textures[i];
}

static ZRDBTexture* ZRDB_CreateBlankTexture(ZRAssetDB* handle, char* name, i32 w, i32 h, ColourU32 fill)
{
	if (w < 1) { w = 4; }
	if (h < 1) { h = 4; }
	i32 bytes = TexGen_BytesFor32BitImage(w, h);
	ColourU32* pixels = (ColourU32*)g_alloc.Allocate(bytes);
	TexGen_SetRGBA(pixels, w, h, fill);
	i32 i = ZRDB_RegisterTexture(handle, name, pixels, bytes, w, h, 0);
	ZRDB_CAST_TO_INTERNAL(handle, db)
	return &db->textures[i];
}

static void ZRDB_GenerateExperiements(ZRAssetDB* db)
{
	ZRDBTexture* base,* emit;
	base = ZRDB_CreateBlankTexture(db, "grid", 32, 32, { 155, 155, 155, 255 });
	TexGen_FillRect((ColourU32*)base->data, 32, 32, { 0, 0 }, { 16, 16 },
		{ 225, 225, 225, 255 });
	TexGen_FillRect((ColourU32*)base->data, 32, 32, { 16, 16 }, { 16, 16 },
		{ 225, 225, 225, 255 });
	
	emit = ZRDB_CreateBlankTexture(db, "grid_emit", 32, 32, { 0, 0, 0, 0 });
	TexGen_FillRect((ColourU32*)emit->data, 32, 32, { 0, 0 }, { 4, 32 },
		{ 225, 225, 0, 255 });
	TexGen_FillRect((ColourU32*)emit->data, 32, 32, { 28, 0 }, { 4, 32 },
		{ 225, 225, 0, 255 });
	db->CreateMaterial(
        db,
        "grid",
        "grid",
        "grid_emit"
    );
}

/**
 * Load in embedded assets. these assets are also default fallbacks.
 */
static void ZRDB_LoadEmbedded(ZRAssetDB* db)
{
    i32 bVerbose = YES;
	MeshData* d;

	d = ZR_Embed_Cube();
    db->LoadMesh(db, ZRDB_MESH_NAME_CUBE, *d, bVerbose);

    d = ZR_Embed_InverseCube();
    db->LoadMesh(db, ZRDB_MESH_NAME_INVERSE_CUBE, *d, bVerbose);

	d = ZR_Embed_Quad();
    db->LoadMesh(db, ZRDB_MESH_NAME_QUAD, *d, bVerbose);
	db->LoadMesh(db, ZRDB_MESH_NAME_DYNAMIC_QUAD, *d, bVerbose);
	
	d = ZR_Embed_Spike();
	// d = ZR_Embed_Cube();
    db->LoadMesh(db, ZRDB_MESH_NAME_SPIKE, *d, bVerbose);

	////////////////////////////////////////
	// Generate
	
	// first texture, index 0 will be fallback default, so purple for debug:
	ZRDB_GenSolidTexture(db, "magenta", { 255, 0, 255, 255 });
	// second texture - charset for bitmap text
	ZRDB_LoadBWImage(db, ZR_Embed_Charset());

	ZRDB_GenSolidTexture(db, ZR_TRANSPARENT_TEX_NAME, { 0, 0, 0, 0});
	ZRDB_GenSolidTexture(db, "white", { 255, 255, 255, 255 });
	ZRDB_GenSolidTexture(db, "black", { 0, 0, 0, 255 });
	ZRDB_GenSolidTexture(db, "grey", { 0, 0, 0, 255 });
	ZRDB_GenSolidTexture(db, "light_grey", { 127, 127, 127, 255 });
	ZRDB_GenSolidTexture(db, "dark_grey", { 55, 55, 55, 255 });
	ZRDB_GenSolidTexture(db, "red", { 255, 0, 0, 255 });
	ZRDB_GenSolidTexture(db, "green", { 0, 255, 0, 255 });
	ZRDB_GenSolidTexture(db, "blue", { 0, 0, 255, 255 });
	ZRDB_GenSolidTexture(db, "yellow", { 255, 255, 0, 255 });
	ZRDB_GenSolidTexture(db, "cyan", { 0, 255, 255, 255 });
	ZRDB_GenSolidTexture(db, "dark_red", { 127, 0, 0, 255 });

	ZRDBTexture* tex = ZRDB_CreateBlankTexture(db, "test", 32, 32, { 25, 25, 25, 255 });
	TexGen_FillRect(
		(ColourU32*)tex->data, 32, 32,
		{ 4, 4 },
		{ 32 - 8, 32 - 8 },
		{ 55, 55, 55, 255 });
	TexGen_FillRect(
		(ColourU32*)tex->data, 32, 32,
		{ 8, 8 },
		{ 32 - 16, 32 - 16 },
		{ 127, 127, 127, 255 });
	
	i32 texSize = 32;
	tex = ZRDB_CreateBlankTexture(db, ZRDB_TEX_NAME_CROSSHAIR, texSize, texSize, { 0, 0, 0, 0 });
	// make a square in the middle
	
	TexGen_FillRect(tex->data, texSize, texSize, { 0, 14 }, { 32, 4 }, COLOUR_U32_GREEN);
	TexGen_FillRect(tex->data, texSize, texSize, { 14, 0 }, { 4, 32 }, COLOUR_U32_GREEN);

	tex = ZRDB_CreateBlankTexture(db, ZRDB_TEX_NAME_SHEET_TEST, texSize, texSize, { 0, 0, 0, 0 });
	TexGen_FillRect(tex->data, texSize, texSize, { 0, 0 }, { 16, 16 }, COLOUR_U32_GREY);
	TexGen_FillRect(tex->data, texSize, texSize, { 16, 0 }, { 16, 16 }, COLOUR_U32_GREY_DARK);
	TexGen_FillRect(tex->data, texSize, texSize, { 0, 16 }, { 16, 16 }, COLOUR_U32_WHITE);
	TexGen_FillRect(tex->data, texSize, texSize, { 16, 16 }, { 16, 16 }, COLOUR_U32_BLACK);

	///////////////////////////////////////////
	// Load texture manifest
    char* textures[] = {
        ZQF_R_DEFAULT_DIFFUSE_TEX
        ,"data/charset.bmp"
    };
    i32 numTextures = sizeof(textures) / sizeof(char*);
    printf("ZRDB - %d textures to load\n", numTextures);
    for (i32 i = 0; i < numTextures; ++i)
    {
        db->LoadTexture(db, textures[i], bVerbose);
    }
    
	// read a bitmap and convert it to b&w header file
	// ZRDBTexture* tex = db->GetTextureByName(db, "data/charset.bmp");
	// ZRDB_GenBWTexture(db, tex);

	///////////////////////////////////////////
    // Create materials
    // Textures need to be loaded before this point!
	///////////////////////////////////////////
    ZRDBMaterial* mat;

	// first material index is always fullbright magenta for debugging
	mat = db->CreateMaterial(
        db,
        ZRDB_DEFAULT_DIFFUSE_MAT_NAME,
        "magenta",
        "magenta");
	
	db->CreateMaterial(
        db,
        ZRDB_DEFAULT_CHARSET_MAT_NAME,
        "charset",
        ZR_TRANSPARENT_TEX_NAME);
    
    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_WORLD,
        // "test",
        ZQF_R_DEFAULT_DIFFUSE_TEX,
        //"cyan",
        ZR_TRANSPARENT_TEX_NAME
    );
	
    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_ENT,
        "red",
        ZR_TRANSPARENT_TEX_NAME
    );

    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_PRJ,
        "dark_red",
        "red"
    );

    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_GFX,
        "yellow",
        "yellow"
    );
    
    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_LASER,
        "red",
        "red"
    );
	
	db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_ENEMY,
        ZQF_R_DEFAULT_DIFFUSE_TEX,
        ZR_TRANSPARENT_TEX_NAME
    );

	db->CreateMaterial(
		db,
		ZRDB_MAT_NAME_LIGHT,
		"white",
		"white"
	);

    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_WORLD_DEBUG,
        ZQF_R_DEFAULT_DIFFUSE_TEX,
        ZQF_R_DEFAULT_DIFFUSE_TEX
    );
	
    db->CreateMaterial(
        db,
        ZRDB_MAT_NAME_CROSSHAIR,
        ZRDB_TEX_NAME_CROSSHAIR,
        ZRDB_TEX_NAME_CROSSHAIR
    );
	//ZRDB_GenerateExperiements(db);
}

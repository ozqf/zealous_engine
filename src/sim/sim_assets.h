
#define SIM_MESH_CUBE "Cube"
#define SIM_MESH_QUAD "Quad"

#define SIM_TEX_CHARSET "textures/charset.bmp"
#define SIM_MAT_CHARSET "game_charset"

#define SIM_MAT_DEFAULT "default"
#define SIM_MAT_WORLD "World"

#define SIM_MAT_IMPACT "Gfx"
#define SIM_MAT_LASER "Laser"



internal void Sim_BuildAssets(ZRAssetDB* db)
{
    printf("SIM - building assets\n");
    
    db->LoadTexture(db, SIM_TEX_CHARSET, YES);
    db->CreateMaterial(db,
        SIM_MAT_CHARSET,
        SIM_TEX_CHARSET,
        "red"
    );
}

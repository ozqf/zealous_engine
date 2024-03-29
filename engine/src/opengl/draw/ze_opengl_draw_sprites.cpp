#include "../ze_opengl_internal.h"

//////////////////////////////////////////////////
// fallback_vert.glsl
//////////////////////////////////////////////////
static const char *world_sprite_vert_text =
    "#version 330\n"
    "\n"
    "uniform mat4 u_projection;\n"
    "uniform mat4 u_modelView;\n"
    "// Vertex Attrib 0\n"
    "layout (location = 0) in vec3 i_position;\n"
    "// // Vertex Attrib 1\n"
    "layout (location = 1) in vec2 i_uv;\n"
    "// // Vertex Attrib 2\n"
    "layout (location = 2) in vec3 i_normal;\n"
    "\n"
    "out vec2 m_texCoord;\n"
    "out vec3 m_normal;\n"
    "out vec3 m_fragPos;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   vec4 positionV4 = vec4(i_position, 1.0);\n"
    "   gl_Position = u_projection * u_modelView * positionV4;\n"
    "   m_texCoord = i_uv;\n"
    "	m_normal = normalize(mat3(u_modelView) * i_normal);\n"
    "	m_fragPos = vec3(u_modelView * positionV4);\n"
    "}\n";

//////////////////////////////////////////////////
// fallback_frag.glsl
//////////////////////////////////////////////////
static const char *world_sprite_frag_text =
    "#version 330\n"
    "\n"
    "uniform vec4 u_colour;\n"
    "uniform sampler2D u_diffuseTex;\n"
    "\n"
    "in vec2 m_texCoord;\n"
    "in vec3 m_normal;\n"
    "in vec3 m_fragPos;\n"
    "\n"
    "out vec4 outputColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   // Output colour * depth - not using UVs or normals yet\n"
    "   vec4 diffuse = texture2D(u_diffuseTex, m_texCoord) * u_colour;\n"
    "   // vec4 diffuse = u_colour;\n"
    "   if (diffuse.w < 0.5) { discard; }\n"
    "   outputColor = diffuse;\n"
    "}\n";

internal i32 g_initialised = NO;
internal ZRShader g_shader = {};
internal ZRMeshHandles g_meshHandles = {};
internal ZRMeshAsset *g_mesh;
// internal ZRMeshData *g_mesh;
internal ZRTexture *tex;
local_persist u32 g_texHandle = 0;

internal void SpriteBatchInit()
{
    g_initialised = YES;
    g_mesh = ZAssets_AllocEmptyMesh("sprite_batch_mesh", 1024 * 3);

    // allocate a texture
    tex = ZAssets_AllocTex(64, 64, "draw_sprites_test_texture");
    // paint onto the texture
    ZGen_FillTexture(tex, {50, 50, 50, 255});
    ZGen_FillTextureRect(tex, COLOUR_U32_GREEN, {0, 0}, {32, 32});
    ZGen_FillTextureRect(tex, COLOUR_U32_RED, {32, 0}, {32, 32});
    ZGen_FillTextureRect(tex, COLOUR_U32_BLUE, {0, 32}, {32, 32});
    ZGen_FillTextureRect(tex, COLOUR_U32_YELLOW, {32, 32}, {32, 32});
    // upload the texture
    ZRGL_UploadTexture((u8 *)tex->data, 64, 64, &g_texHandle);

    // shader
    ZRGL_CreateProgram(
        world_sprite_vert_text, world_sprite_frag_text, "sprite_batch", &g_shader);
}

///////////////////////////////////////////////////////////
// Create sprite mesh and submit for drawing
///////////////////////////////////////////////////////////
ze_external void ZRDraw_SpriteBatch(
    ZRDrawCmdSpriteBatch* batch, M4x4* view, M4x4* projection)
{
    if (!g_initialised)
    {
        SpriteBatchInit();
    }
    ZE_PRINTF("Draw sprite batch - %d objects\n", batch->numItems);
    g_mesh->data.Clear();
    
    ColourF32 colour = COLOUR_F32_WHITE;
    f32 lerp = 0;
    for (i32 i = 0; i < batch->numItems; ++i)
    {
        ZRSpriteBatchItem* item = &batch->items[i];
        colour = item->colour;
        ZGen_AddSriteGeoXY(&(g_mesh->data), item->pos, item->size, item->uvMin, item->uvMax, item->radians);
    }
    Vec4 vecColour = ColourF32ToVec4(colour);
    //printf("Draw quad colour from %.3f, %.3f, %.3f to %.3f, %.3f, %.3f\n",
    //    colour.r, colour.g, colour.b,
    //    vecColour.x, vecColour.y, vecColour.z);
    glUseProgram(g_shader.handle);

    // upload
    ZRGL_UploadMesh(&g_mesh->data, &g_meshHandles, 0);
    
    M4x4_CREATE(modelView)
    M4x4_CREATE(model)

    M4x4_Multiply(modelView.cells, view->cells, modelView.cells);
    M4x4_Multiply(modelView.cells, model.cells, modelView.cells);

    ZR_SetProgM4x4(g_shader.handle, "u_modelView", modelView.cells);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", projection->cells);
    ZR_SetProgVec4f(g_shader.handle, "u_colour", vecColour);

    u32 texHandle = ZRGL_GetTextureHandle(batch->textureId);

    ZR_PrepareTextureUnit2D(g_shader.handle, GL_TEXTURE0, 0, "u_diffuseTex", texHandle, 0);
    CHECK_GL_ERR

    // set frag output colour
    // ZR_SetProgVec4f(g_shader.handle, "u_colour", { 1, 1, 1, 1});
    CHECK_GL_ERR

    glBindVertexArray(g_meshHandles.vao);
    CHECK_GL_ERR
    glDrawArrays(GL_TRIANGLES, 0, g_meshHandles.vertexCount);
    CHECK_GL_ERR
}

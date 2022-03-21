#include "../ze_opengl_internal.h"

//////////////////////////////////////////////////
// fallback_vert.glsl
//////////////////////////////////////////////////
static const char *world_cube_vert_text =
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
static const char *world_cube_frag_text =
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
    "   if (diffuse.w < 0.5) { discard; }\n"
    "   outputColor = diffuse;\n"
    "}\n";

//////////////////////////////////////////////////
// external functions
//////////////////////////////////////////////////
ze_external void ZRGL_Debug_DrawWorldCubeTest()
{
    local_persist i32 initialised = NO;

    local_persist ZRShader g_shader = {};
    local_persist u32 g_meshId = 0;

    local_persist u32 g_quadVAO;
    local_persist u32 g_quadVBO;
    local_persist u32 g_diffuseTex;

    local_persist M4x4 modelMatrix;
    local_persist M4x4 viewMatrix;
    local_persist M4x4 prjMatrix;

    local_persist M4x4 modelView;

    const Vec4 colour = { 1, 1, 1, 1 };

    const i32 numTris = 12;
    const i32 numVerts = 36;
    // 6 faces, 2 tris per face, 3 verts per tri
    // == 36 verts * 3 components each == 108
    const f32 cubeVerts[numVerts * 3] =
    {
        // face 0 (front)
        -0.5, -0.5,  0.5,
         0.5, -0.5,  0.5,
         0.5,  0.5,  0.5,
        -0.5, -0.5,  0.5,
         0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,
        // face 0.5 (right side)
         0.5, -0.5,  0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
         0.5, -0.5,  0.5,
         0.5,  0.5, -0.5,
         0.5,  0.5,  0.5,
        // face 2 (back)
         0.5, -0.5, -0.5,
        -0.5, -0.5, -0.5,
        -0.5,  0.5, -0.5,
         0.5, -0.5, -0.5,
        -0.5,  0.5, -0.5,
         0.5,  0.5, -0.5,
        // face 3 (left side)
        -0.5, -0.5, -0.5,
        -0.5, -0.5,  0.5,
        -0.5,  0.5,  0.5,
        -0.5, -0.5, -0.5,
        -0.5,  0.5,  0.5,
        -0.5,  0.5, -0.5,
        // face 4 (top)
        -0.5,  0.5,  0.5,
         0.5,  0.5,  0.5,
         0.5,  0.5, -0.5,
        -0.5,  0.5,  0.5,
         0.5,  0.5, -0.5,
        -0.5,  0.5, -0.5,
        // face 5 (bottom)
        -0.5, -0.5, -0.5,
         0.5, -0.5, -0.5,
         0.5, -0.5,  0.5,
        -0.5, -0.5, -0.5,
         0.5, -0.5,  0.5,
        -0.5, -0.5,  0.5
    };

    const f32 cubeUVs[] =
    {
        // face 0
        0, 0,
        1, 0,
        1, 1,
        0, 0,
        1, 1,
        0, 1,
        // face 1
        0, 0,
        1, 0,
        1, 1,
        0, 0,
        1, 1,
        0, 1,
        // face 2
        0, 0,
        1, 0,
        1, 1,
        0, 0,
        1, 1,
        0, 1,
        // face 3
        0, 0,
        1, 0,
        1, 1,
        0, 0,
        1, 1,
        0, 1,
        // face 4
        0, 0,
        1, 0,
        1, 1,
        0, 0,
        1, 1,
        0, 1,
        // face 5
        0, 0,
        1, 0,
        1, 1,
        0, 0,
        1, 1,
        0, 1
    };

    const f32 cubeNormals[] =
    {
        // face 0 (front)
        0,  0,  1,
        0,  0,  1,
        0,  0,  1,
        0,  0,  1,
        0,  0,  1,
        0,  0,  1,
        // face 1 (right side)
        1,  0,  0,
        1,  0,  0,
        1,  0,  0,
        1,  0,  0,
        1,  0,  0,
        1,  0,  0,
        // face 2 (back)
        0,  0, -1,
        0,  0, -1,
        0,  0, -1,
        0,  0, -1,
        0,  0, -1,
        0,  0, -1,
        // face 3 (left side)
        -1,  0,  0,
        -1,  0,  0,
        -1,  0,  0,
        -1,  0,  0,
        -1,  0,  0,
        -1,  0,  0,
        // face 4 (top)
        0,  1,  0,
        0,  1,  0,
        0,  1,  0,
        0,  1,  0,
        0,  1,  0,
        0,  1,  0,
        // face 5 (bottom)
        0, -1,  0,
        0, -1,  0,
        0, -1,  0,
        0, -1,  0,
        0, -1,  0,
        0, -1,  0
    };

    if (!initialised)
    {
        initialised = YES;

        M4x4_SetToIdentity(modelMatrix.cells);
        M4x4_SetToIdentity(viewMatrix.cells);
        M4x4_SetToIdentity(modelView.cells);
        M4x4_RotateByAxis(modelView.cells, 45.f * DEG2RAD, 1, 0, 0);
        M4x4_SetToIdentity(prjMatrix.cells);

        ZE_SetupDefault3DProjection(prjMatrix.cells, Window_GetInfo().aspect);

        glGenVertexArrays(1, &g_quadVAO);
        glGenBuffers(1, &g_quadVBO);

        // bind vao/vbo
        glBindVertexArray(g_quadVAO);
        CHECK_GL_ERR
        glBindBuffer(GL_ARRAY_BUFFER, g_quadVBO);
        CHECK_GL_ERR

        // calculate size
        i32 numVertBytes = (numVerts * 3) * sizeof(f32);
        i32 numUVBytes = (numVerts * 2) * sizeof(f32);
        i32 numNormalBytes = (numVerts * 3) * sizeof(f32);
        i32 totalBytes = numVertBytes + numUVBytes + numNormalBytes;

        GLenum vboUsage = GL_STATIC_DRAW; // (other option could be GL_DYNAMIC_DRAW)

        // Allocate buffer for all three arrays verts-normals-uvs
        // send null ptr for data, we're not uploading yet
        glBufferData(GL_ARRAY_BUFFER, totalBytes, NULL, vboUsage);

        zeSize vertOffset = 0;
        zeSize uvOffset = numVertBytes;
        zeSize normalOffset = numVertBytes + numUVBytes;

        // BUFFER: - All Verts | All Normals | All Uvs -
        glBufferSubData(GL_ARRAY_BUFFER, vertOffset, numVertBytes, cubeVerts);
        glBufferSubData(GL_ARRAY_BUFFER, uvOffset, numUVBytes, cubeUVs);
        glBufferSubData(GL_ARRAY_BUFFER, normalOffset, numNormalBytes, cubeNormals);

        // enable use of static data
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        CHECK_GL_ERR
        GLenum glDataType = GL_FLOAT;
        // setup how to read the static data sections
        glVertexAttribPointer(0, 3, glDataType, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 2, glDataType, GL_FALSE, 0, (void *)uvOffset);
        glVertexAttribPointer(2, 3, glDataType, GL_FALSE, 0, (void *)normalOffset);
        CHECK_GL_ERR

        // #define ZRGL_DATA_ATTRIB_VERTICES 0
        // #define ZRGL_DATA_ATTRIB_UVS 1
        // #define ZRGL_DATA_ATTRIB_NORMALS 2

        ZRGL_CreateProgram(
            world_cube_vert_text, world_cube_frag_text, "fallback", &g_shader);
        
        // upload a texture
        ZRTexture *tex = ZAssets_AllocTex(64, 64, "draw_world_cube_test_texture");
        ZGen_FillTexture(tex, { 50, 50, 50, 255 });
        ZGen_SetPixel(tex, COLOUR_U32_RED, 1, 1);
        ZGen_SetPixel(tex, COLOUR_U32_RED, 1, 62);
        ZGen_SetPixel(tex, COLOUR_U32_RED, 62, 1);
        ZGen_SetPixel(tex, COLOUR_U32_RED, 62, 62);

        ZGen_FillTextureRect(tex, COLOUR_U32_GREEN, { 16, 16}, { 32, 32 });

        ZRGL_UploadTexture((u8*)tex->data, 64, 64, &g_diffuseTex);
    }
    printf("Draw world cube test\n");
    /////////////////////////////////////////////////////////////
    // Clear
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERR
    glClearColor(0, 0, 0, 1);
    glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERR
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERR

    glUseProgram(g_shader.handle);

    // set vert matrices
    float delta = 1.f / 60.f;
    // move away from camera otherwise we'll draw IN the cube
    modelView.posZ = -2.f;
    
    M4x4_RotateY(modelView.cells, (90.f * DEG2RAD) * delta);
    ZR_SetProgM4x4(g_shader.handle, "u_projection", prjMatrix.cells);
    ZR_SetProgM4x4(g_shader.handle, "u_modelView", modelView.cells);

    ZR_PrepareTextureUnit2D(g_shader.handle, GL_TEXTURE0, 0, "u_diffuseTex", g_diffuseTex, 0);

    // set frag output colour
    ZR_SetProgVec4f(g_shader.handle, "u_colour", colour);

    glBindVertexArray(g_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, numVerts);
}

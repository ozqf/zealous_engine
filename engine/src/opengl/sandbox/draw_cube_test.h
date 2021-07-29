#include "../ze_opengl_internal.h"

//////////////////////////////////////////////////
// fallback_frag.glsl
//////////////////////////////////////////////////
static const char *fallback_frag_text =
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
    "#if 1 // Output colour\n"
    "   outputColor = u_colour;\n"
    "#endif\n"
    "#if 1 // Output depth\n"
    "   float depthValue = gl_FragCoord.z;\n"
    "   outputColor = vec4(u_colour.x * depthValue, u_colour.y * depthValue, u_colour.z * depthValue, 1);\n"
    "#endif\n"
    "   //outputColor = u_colour;\n"
    "#if 0 // output texture\n"
    "   vec4 diffuse = texture2D(u_diffuseTex, m_texCoord) * u_colour;\n"
    "   if (diffuse.w < 0.5) { discard; }\n"
    "   outputColor = diffuse;\n"
    "#endif\n"
    "}\n";
//////////////////////////////////////////////////
// fallback_vert.glsl
//////////////////////////////////////////////////
static const char *fallback_vert_text =
    "#version 330\n"
    "\n"
    "uniform mat4 u_projection;\n"
    "uniform mat4 u_modelView;\n"
    "// uniform mat4 u_mvp;\n"
    "// uniform mat4 u_depthMVP;\n"
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
// external functions
//////////////////////////////////////////////////
ze_external void ZRGL_Debug_DrawCubeTest()
{
    local_persist i32 initialised = NO;

    local_persist ZRShader g_shader = {};
    local_persist u32 g_meshId = 0;

    local_persist u32 g_quadVAO;
    local_persist u32 g_quadVBO;

    local_persist f32 modelMatrix[16];
    local_persist f32 viewMatrix[16];
    local_persist f32 prjMatrix[16];

    // 2 tris == 6 verts * 3 components each == 18
    const f32 g_quadVerts[18] =
    {
        -0.5, -0.5, 0,
        0.5, -0.5, 0,
        0.5, 0.5, 0,

        -0.5, -0.5, 0,
        0.5, 0.5, 0,
        -0.5, 0.5, 0
    };

    const f32 g_prim_quadUVs[] =
    {
        0, 0,
        1, 0,
        1, 1,

        0, 0,
        1, 1,
        0, 1
    };

    const f32 g_prim_quadNormals[] =
    {
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,

        0, 0, -1,
        0, 0, -1,
        0, 0, -1
    };

    if (!initialised)
    {
        initialised = YES;

        glGenVertexArrays(1, &g_quadVAO);
        glGenBuffers(1, &g_quadVBO);

        // bind vao/vbo
        glBindVertexArray(g_quadVAO);
        CHECK_GL_ERR
        glBindBuffer(GL_ARRAY_BUFFER, g_quadVBO);
        CHECK_GL_ERR

        // calculate size
        i32 numVertBytes = 18 * sizeof(f32);
        i32 numUVBytes = 12 * sizeof(f32);
        i32 numNormalBytes = 18 * sizeof(f32);
        i32 totalBytes = numVertBytes + numUVBytes + numNormalBytes;

        GLenum vboUsage = GL_STATIC_DRAW; // (other option could be GL_DYNAMIC_DRAW)

        // Allocate buffer for all three arrays verts-normals-uvs
        // send null ptr for data, we're not uploading yet
        glBufferData(GL_ARRAY_BUFFER, totalBytes, NULL, vboUsage);

        i32 vertOffset = 0;
        i32 uvOffset = numVertBytes;
        i32 normalOffset = numVertBytes + numUVBytes;

        // BUFFER: - All Verts | All Normals | All Uvs -
        glBufferSubData(GL_ARRAY_BUFFER, vertOffset, numVertBytes, g_quadVerts);
        glBufferSubData(GL_ARRAY_BUFFER, uvOffset, numUVBytes, g_prim_quadUVs);
        glBufferSubData(GL_ARRAY_BUFFER, normalOffset, numNormalBytes, g_prim_quadNormals);

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
            fallback_vert_text, fallback_frag_text, "fallback", 0, NO, &g_shader);
    }

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
    glBindVertexArray(g_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

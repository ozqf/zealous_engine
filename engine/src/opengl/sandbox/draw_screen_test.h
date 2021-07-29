#include "../../../../lib/glad/glad.h"

#ifndef local_persist
#define local_persist static
#endif

//////////////////////////////////////////////////
// Draw a red quad to screenspace
//////////////////////////////////////////////////
ze_external void ZRGL_Debug_DrawCubeTest()
{
    //////////////////////////////////////////////////
    // data
    local_persist const char *pass_vert_text =
        "#version 330\n"
        "layout (location = 0) in vec3 i_position;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(i_position, 1);\n"
        "}\n";

    local_persist const char *debug_frag_text =
        "#version 330\n"
        "out vec4 outputColour;\n"
        "void main()\n"
        "{\n"
        "   outputColour = vec4(1, 0, 0, 1);\n"
        "}\n";

    // 2 tris == 6 verts * 3 components each == 18
    const float quadVerts[18] =
    {
        -0.5, -0.5, 0,
        0.5, -0.5, 0,
        0.5, 0.5, 0,

        -0.5, -0.5, 0,
        0.5, 0.5, 0,
        -0.5, 0.5, 0
    };
    //////////////////////////////////////////////////
    // vars
    local_persist int initialised = NO;
    local_persist unsigned int shaderHandle;
    local_persist unsigned int vaoHandle;
    local_persist unsigned int vboHandle;

    //////////////////////////////////////////////////
    // setup
    if (!initialised)
    {
        initialised = YES;

        // gen and bind slots handles for vao/vbo
        glGenVertexArrays(1, &vaoHandle);
        glGenBuffers(1, &vboHandle);
        glBindVertexArray(vaoHandle);
        glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
        
        // calculate size
        int numVertBytes = 18 * sizeof(float);

        // Allocate buffer for all three arrays verts-normals-uvs
        // send null ptr for data, we're not uploading yet
        // vbo usage could be GL_DYNAMIC_DRAW if we want to update geometry)
        glBufferData(GL_ARRAY_BUFFER, numVertBytes, NULL, GL_STATIC_DRAW);

        // upload vert data - only verts so offset 0, all bytes
        glBufferSubData(GL_ARRAY_BUFFER, 0, numVertBytes, quadVerts);
        
        // enable use of static data - just one attrib for verts, no UVs or normals
        glEnableVertexAttribArray(0);
        
        // setup how to read the static data section (would be more for UVs and normals)
        // step by (3 * sizeof(float)) for each item
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // compile shader - no error checking whatsoever (:

        // Vertex shader
        GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShaderId, 1, &pass_vert_text, NULL);
        glCompileShader(vertexShaderId);

        // Fragment shader
        GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShaderId, 1, &debug_frag_text, NULL);
        glCompileShader(fragmentShaderId);

        // Create program
        shaderHandle = glCreateProgram();
        glAttachShader(shaderHandle, vertexShaderId);
        glAttachShader(shaderHandle, fragmentShaderId);

        // link program
        glLinkProgram(shaderHandle);
    }
    //////////////////////////////////////////////////
    // draw
    
    // clear
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // bind
    glUseProgram(shaderHandle);
    glBindVertexArray(vaoHandle);

    // draw
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // reset
    glEnable(GL_DEPTH_TEST);
}

#include "../../../../lib/glad/glad.h"

//////////////////////////////////////////////////
// Draw a red quad to screenspace
//////////////////////////////////////////////////
extern "C" void OpenglTest_DrawScreenSpaceQuad()
{
    //////////////////////////////////////////////////
    // data

    // vert shader - literally do nothing, just pass vert position
    // as final screen coordinate
    static const char *pass_vert_text =
        "#version 330\n"
        "layout (location = 0) in vec3 i_position;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(i_position, 1);\n"
        "}\n";

    // fragment shader - set output to given colour
    static const char *debug_frag_text =
        "#version 330\n"
        "out vec4 outputColour;\n"
        "void main()\n"
        "{\n"
        "   outputColour = vec4(1, 0, 0, 1);\n"
        "}\n";

    // 2 tris == 6 verts * 3 components each == 18
	// no model/view transformation, so coords are screen coords here.
	// screen space is -1 to 1 on x, y and z.
    const float quadVerts[18] =
    {
        -0.5, -0.5, 0, // bottom left
        0.5, -0.5, 0, // bottom right
        0.5, 0.5, 0, // top right

        -0.5, -0.5, 0, // bottom left
        0.5, 0.5, 0, // top right
        -0.5, 0.5, 0 // top left
    };
    //////////////////////////////////////////////////
    // vars - handles to GPU objects
    static int bInitialised = NO;
    static unsigned int shaderHandle;
    static unsigned int vaoHandle;
    static unsigned int vboHandle;

    //////////////////////////////////////////////////
    // setup
    if (!bInitialised)
    {
        bInitialised = YES;

        // gen and bind handles for "vertex Array Object" and "Vertex Buffer Object"
        // VAO is a collection of multiple vertex sets. we're only using one here.
        glGenVertexArrays(1, &vaoHandle);
        glGenBuffers(1, &vboHandle);
        glBindVertexArray(vaoHandle);
        glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
        
        // calculate size of mesh data
        int numVertBytes = 18 * sizeof(float);

        // Allocate buffer for mesh data - only vertices here, would normally include UVs/Normals
        // send null ptr for data, we're not uploading yet
        // vbo usage could be GL_DYNAMIC_DRAW if we want to update geometry)
        glBufferData(GL_ARRAY_BUFFER, numVertBytes, NULL, GL_STATIC_DRAW);

        // upload vert data - only verts so start copy at offset 0, all bytes
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
    glDisable(GL_DEPTH_TEST); // not interested in depth buffer for this
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind gpu objects
    glUseProgram(shaderHandle);
    glBindVertexArray(vaoHandle);

    // draw
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

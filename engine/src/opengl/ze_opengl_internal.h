#ifndef ZE_OPENGL_INTERNAL_H
#define ZE_OPENGL_INTERNAL_H

#include "../../internal_headers/zengine_internal.h"

// third party
#include "../../../lib/glad/glad.h"
#include "../../../lib/glfw3_vc2015/glfw3.h"

// Finding this makes it easier to hunt down specific error locations
// vs using opengl's error callback...
#define CHECK_GL_ERR                                                                                           \
    {                                                                                                          \
        GLenum checkGlErrVal;                                                                                  \
        while ((checkGlErrVal = glGetError()) != GL_NO_ERROR)                                                  \
        {                                                                                                      \
            char *checkGlErrType = "Unknown";                                                                  \
            switch (checkGlErrVal)                                                                             \
            {                                                                                                  \
            case 0x500:                                                                                        \
                checkGlErrType = "GL_INVALID_ENUM";                                                            \
                break;                                                                                         \
            case 0x501:                                                                                        \
                checkGlErrType = "GL_INVALID_VALUE";                                                           \
                break;                                                                                         \
            case 0x502:                                                                                        \
                checkGlErrType = "GL_INVALID_OPERATION";                                                       \
                break;                                                                                         \
            }                                                                                                  \
            printf("GL Error 0x%X (%s) at %s line %d\n", checkGlErrVal, checkGlErrType, __FILE__, (__LINE__)); \
        }                                                                                                      \
    }

///////////////////////////////////////////////////////
// Internal datatypes
///////////////////////////////////////////////////////

struct ZRShader
{
    i32 handle;      // considered invalid if this is 0
    i32 drawObjType; // considered invalid if this is 0
    i32 bBatchable;
    char *name;
};

/**
 * handles for executing a draw call
 */
struct ZRMeshDrawHandles
{
    i32 vao;
    i32 vertCount;
    i32 diffuseHandle;
    i32 emissiveHandle;
};

///////////////////////////////////////////////////////
// Internal functions
///////////////////////////////////////////////////////
extern "C" ErrorCode ZRGL_CreateProgram(
    const char *vertexShader,
    const char *fragmentShader,
    char *shaderName,
    const i32 drawObjType,
    const i32 bIsBatchable,
    ZRShader *result);

extern "C" void ZRGL_PrintShaderCompileLog(GLuint shaderId);

//////////////////////////////////////////////////
// fallback_frag.glsl
//////////////////////////////////////////////////
static const char* fallback_frag_text =
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
"#if 0 // Output depth\n"
"   float depthValue = gl_FragCoord.z;\n"
"   outputColor = vec4(u_colour.x * depthValue, u_colour.y * depthValue, u_colour.z * depthValue, 1);\n"
"#endif\n"
"   //outputColor = u_colour;\n"
"#if 1 // output texture\n"
"   vec4 diffuse = texture2D(u_diffuseTex, m_texCoord) * u_colour;\n"
"   if (diffuse.w < 0.5) { discard; }\n"
"   outputColor = diffuse;\n"
"#endif\n"
"}\n"
;
//////////////////////////////////////////////////
// fallback_vert.glsl
//////////////////////////////////////////////////
static const char* fallback_vert_text =
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
"}\n"
;
#endif // ZE_OPENGL_INTERNAL_H

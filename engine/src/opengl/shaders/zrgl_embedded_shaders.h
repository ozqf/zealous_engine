#ifndef ZRGL_EMBEDDED_SHADERS_H
#define ZRGL_EMBEDDED_SHADERS_H
/* This file is automatically generated */
//////////////////////////////////////////////////
// draw_single_mesh_frag.glsl
//////////////////////////////////////////////////
static const char* draw_single_mesh_frag_text =
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
"#if 1 // output texture\n"
"   vec4 diffuse = texture2D(u_diffuseTex, m_texCoord);\n"
"   diffuse *= u_colour;\n"
"   outputColor = diffuse;\n"
"#endif\n"
"\n"
"#if 0\n"
"   outputColor = vec4(1, 1, 1, 1);\n"
"#endif\n"
"#if 0\n"
"   outputColor = vec4(m_texCoord.x, m_texCoord.y, 1, 1);\n"
"#endif\n"
"#if 0\n"
"   outputColor = u_colour;\n"
"#endif\n"
"#if 0 // Output depth\n"
"   float depthValue = gl_FragCoord.z;\n"
"   outputColor = vec4(u_colour.x * depthValue, u_colour.y * depthValue, u_colour.z * depthValue, 1);\n"
"#endif\n"
"#if 0 // output texture with boolean transparency\n"
"   vec4 diffuse = texture2D(u_diffuseTex, m_texCoord);\n"
"   if (diffuse.w < 0.5) { discard; }\n"
"   outputColor = diffuse;\n"
"#endif\n"
"}\n"
;
//////////////////////////////////////////////////
// draw_single_mesh_vert.glsl
//////////////////////////////////////////////////
static const char* draw_single_mesh_vert_text =
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
"}\n"
"\n"
;

#endif // ZRGL_EMBEDDED_SHADERS_H

#version 330

uniform mat4 u_projection;
uniform mat4 u_modelView;
// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// // Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// // Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragPos;

void main()
{
   vec4 positionV4 = vec4(i_position, 1.0);
   gl_Position = u_projection * u_modelView * positionV4;
   m_texCoord = i_uv;
	m_normal = normalize(mat3(u_modelView) * i_normal);
	m_fragPos = vec3(u_modelView * positionV4);
}

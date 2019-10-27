
#version 330 core

// Program data inputs
uniform mat4 u_projection;
uniform mat4 u_view;

// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

// outputs to fragment shader
out vec3 m_texCoord;

void main()
{
	m_texCoord = i_position;
	gl_Position = u_projection * u_view * vec4(i_position, 1.0);
}

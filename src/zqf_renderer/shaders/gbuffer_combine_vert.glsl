#version 330 core

// Program data inputs
uniform mat4 u_projection;
uniform mat4 u_modelView;

// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// Vertex Attrib 1
layout (location = 1) in vec2 i_uv;

// outputs to fragment shader
out vec2 m_texCoord;

void main()
{
	vec4 positionV4 = vec4(i_position, 1.0);
	gl_Position = u_projection*u_modelView*positionV4;
    m_texCoord = i_uv;
}

#version 330 core

uniform mat4 u_projection;
uniform mat4 u_modelView;

// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

void main()
{
    gl_Position = u_projection * u_modelView * vec4(i_position, 1.0);
}
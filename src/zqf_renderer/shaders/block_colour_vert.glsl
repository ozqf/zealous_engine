#version 330
uniform mat4 u_projection;
uniform mat4 u_modelView;
// inputs, unique for each vert
layout (location = 0) in vec3 inPosition;
void main()
{
   gl_Position = u_projection*u_modelView*vec4(inPosition, 1.0);
}
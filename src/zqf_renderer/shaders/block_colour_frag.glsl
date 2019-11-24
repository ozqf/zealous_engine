#version 330

uniform vec3 u_colour;
out vec4 outputColor;

void main()
{
   //outputColor = vec4(1.0, 1.0, 1.0, 1.0);
   outputColor = vec4(u_colour, 1.0);
}
#version 330

uniform vec3 diffuseColour;
out vec4 outputColor;

void main()
{
   outputColor = vec4(1.0, 1.0, 1.0, 1.0);
   outputColor = vec4(diffuseColour, 1.0);
}
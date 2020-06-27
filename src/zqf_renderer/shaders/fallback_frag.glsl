#version 330

uniform vec4 u_colour;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

out vec4 outputColor;

void main()
{
   float depthValue = gl_FragCoord.z;
   outputColor = vec4(u_colour.x * depthValue, u_colour.y * depthValue, u_colour.z * depthValue, 1);
   //outputColor = u_colour;
}
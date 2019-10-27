#version 330 core

// input from vert shader
in vec2 m_texCoord;
in vec3 m_fragPos;
in vec3 m_normal;
// flat - must match signature of output from vert shader
flat in int m_instanceID;
flat in ivec2 m_dataPixel;
flat in int m_dataPixelIndex;
//int m_instanceColour;

// final output colour
out vec4 outputColor;

// Program data inputs
uniform sampler2D u_dataTex2D;

uniform int u_dataTexWidth;

uniform int u_debugMode;

void main()
{
    int dataPixel = m_dataPixelIndex;
    outputColor = texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0);
}
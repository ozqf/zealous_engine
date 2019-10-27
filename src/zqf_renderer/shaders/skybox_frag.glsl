#version 330 core

// input from vert shader
in vec3 m_texCoord;
out vec4 outputColor;

// Program data inputs
uniform samplerCube u_cubeMap;
uniform sampler2D u_debug;

void main()
{
	outputColor = texture(u_cubeMap, m_texCoord);
    //outputColor = vec4(m_texCoord.x, m_texCoord.y, m_texCoord.z, 1);
    //outputColor = vec4(1, 0, 1, 1);
    //outputColor = texture(u_debug, vec2(m_texCoord.x, m_texCoord.y));
}

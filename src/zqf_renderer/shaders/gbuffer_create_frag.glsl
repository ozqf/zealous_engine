#version 330

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColour;

uniform sampler2D u_colourTex;

in vec3 m_worldPos;
in vec2 m_texCoord;
in vec3 m_normal;

void main()
{
    gPosition = m_worldPos;
    gNormal = m_normal;
    gColour = texture2D(u_colourTex, m_texCoord);
}
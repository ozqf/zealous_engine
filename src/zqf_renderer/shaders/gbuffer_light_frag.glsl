#version 330 core

// VS inputs
in vec3 m_worldPos;
in vec2 m_texCoord;
in vec3 m_normal;

// gBuffer
uniform sampler2D u_colourTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_positionTex;

out vec4 outputColor;

void main()
{
    // outputColor = vec4(1, 1, 1, 1);

    vec4 colour = texture2D(u_colourTex, m_texCoord);
    outputColor = colour * 0.5;
}
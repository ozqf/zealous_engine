#version 330 core

// VS inputs
in vec3 m_worldPos;
in vec2 m_texCoord;
in vec3 m_normal;
in vec4 m_screenPos;

// gBuffer
uniform sampler2D u_colourTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_positionTex;

uniform int u_windowWidth;
uniform int u_windowHeight;

out vec4 outputColor;

void main()
{
    vec2 screenSize = vec2(u_windowWidth, u_windowHeight);
    vec2 screenPos = gl_FragCoord.xy / screenSize;

    vec4 colour = texture2D(u_colourTex, screenPos);
    outputColor = colour;// * 0.5;
}
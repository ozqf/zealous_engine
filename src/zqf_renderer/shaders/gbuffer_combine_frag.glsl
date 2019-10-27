#version 330

// gBuffer
uniform sampler2D u_colourTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_positionTex;

in vec2 m_texCoord;
out vec4 outputColor;

void main()
{
    vec3 colour = vec3(texture2D(u_colourTex, m_texCoord));
    vec3 normal = vec3(texture2D(u_normalTex, m_texCoord));
    outputColor = vec4((colour * 0.5) + (normal * 0.5), 1);
    //outputColor = vec4(m_texCoord, 0, 1);
}
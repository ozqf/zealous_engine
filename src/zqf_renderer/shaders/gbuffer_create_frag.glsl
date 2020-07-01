#version 330

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColour;
layout (location = 3) out vec4 gEmission;

uniform sampler2D u_colourTex;
uniform sampler2D u_emissionTex;

in vec3 m_worldPos;
in vec2 m_texCoord;
in vec3 m_normal;

void main()
{
    vec4 colour = texture2D(u_colourTex, m_texCoord);
    vec4 emission = texture2D(u_emissionTex, m_texCoord);
    if (colour.w > 0.5)
    {
        gPosition = m_worldPos;
        gNormal = m_normal;
        gColour = colour;
        //gEmission = emission;
        gEmission = vec4(0, 0, 0, 1);
    }
    else
    {
        discard;
    }
}
#version 330

// gBuffer
uniform sampler2D u_positionTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_colourTex;

// light params
uniform vec3 u_lightWorldPos;
uniform vec3 u_lightWorldDir;
uniform vec3 u_lightColour;
uniform float u_lightRange;

in vec2 m_texCoord;
out vec4 outputColor;

void main()
{
    vec3 colour = vec3(texture2D(u_colourTex, m_texCoord));
    vec3 normal = vec3(texture2D(u_normalTex, m_texCoord));
    vec3 fragWorldPos = vec3(texture2D(u_positionTex, m_texCoord));
    //vec3 fragWorldPos = vec3(texture2D(u_normalTex, m_texCoord));
    float dist = distance(fragWorldPos, u_lightWorldPos);
    #if 0
    outputColor = vec4(fragWorldPos, 1);
    //outputColor = vec4(normal, 1);
    //outputColor = vec4(colour, 1);
    #endif
    #if 0
    if (length(fragWorldPos) > 10)
    {
        outputColor = vec4(0, 1, 0, 1);
    }
    else
    {
        outputColor = vec4(1, 0, 0, 1);
    }
    #endif
    #if 1
    if (dist > u_lightRange)
    {
        discard;
    }
    //outputColor = vec4((colour * 0.5) + (normal * 0.5), 1);
    //outputColor = vec4(m_texCoord, 0, 1);
    outputColor = vec4(colour, 1);
    #endif
}
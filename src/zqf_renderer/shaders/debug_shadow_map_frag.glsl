#version 330

uniform sampler2D u_diffuseTex;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

out vec4 outputColor;

void main()
{
    //vec4 colour = texture2D(u_diffuseTex, m_texCoord);
    //outputColor = vec4(colour.z, colour.z, colour.z, 1);
    // 
    //float depthValue = texture(u_diffuseTex, m_texCoord).r;
    //outputColor = vec4(vec3(depthValue), 1.0);
    vec4 colour = texture2D(u_diffuseTex, m_texCoord);
    if (colour.w < 0.9)
    {
        discard;
    }
    outputColor = colour;
}
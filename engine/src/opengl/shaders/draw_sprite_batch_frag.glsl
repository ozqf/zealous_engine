#version 330

uniform sampler2D u_diffuseTex;
uniform int u_instanceCount;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

// flat - no interpolation
flat in int m_instanceID;

out vec4 outputColor;

void main()
{
    vec4 diffuse = texture2D(u_diffuseTex, m_texCoord);
    float r = float(m_instanceID) / float(u_instanceCount);
    float g = 1.0 - float(m_instanceID) / float(u_instanceCount);
    outputColor = vec4(diffuse.x * r, diffuse.y * g, diffuse.b, 1);
}

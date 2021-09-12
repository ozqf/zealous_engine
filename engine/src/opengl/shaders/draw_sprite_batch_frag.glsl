#version 330

uniform sampler2D u_diffuseTex;
uniform sampler2D u_dataTexture;
uniform int u_instanceCount;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

// flat - no interpolation
flat in int m_instanceID;
flat in vec4 m_colour;

out vec4 outputColor;

void main()
{
    #if 1
    vec4 diffuse = texture2D(u_diffuseTex, m_texCoord);
    outputColor = diffuse;
    #endif

    #if 0
    vec4 diffuse = texture2D(u_diffuseTex, m_texCoord);
    float r = float(m_instanceID) / float(u_instanceCount);
    float g = 1.0 - float(m_instanceID) / float(u_instanceCount);
    outputColor = vec4(diffuse.x * r, diffuse.y * g, diffuse.b, 1);
    #endif
    
    #if 0
    // vec4 block = vec4(1, 1, 1, 1);
    // vec4 data1 = texelFetch(u_dataTexture, ivec2(m_instanceID, 0);

    // vec4 diffuse = vec4(texelFetch(u_dataTexture, ivec2(m_instanceID, 0), 0));
    // vec4 diffuse = vec4(texelFetch(u_dataTexture, ivec2(0, 0), 0));
    // outputColor = (block * 0.5) + (m_colour * 0.5);
    outputColor = m_colour;
    #endif
}

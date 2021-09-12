#version 330

uniform int u_instanceCount;

// flat - no interpolation
flat in int m_instanceID;

out vec4 outputColor;

void main()
{
    float r = float(m_instanceID) / float(u_instanceCount);
    float g = 1.0 - float(m_instanceID) / float(u_instanceCount);
    outputColor = vec4(r, g, 1, 1);
}

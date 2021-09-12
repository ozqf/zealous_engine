#version 330

#if 1
uniform mat4 u_projection;
uniform int u_instanceCount;

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;

out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragPos;
// flat - no interpolation
flat out int m_instanceID;

void main()
{
    // pass instance to frag shader
    m_instanceID = gl_InstanceID;

    mat4 u_modelView = u_projection;

    u_modelView[0][0] *= 0.5;
    u_modelView[1][1] *= 0.5;
    u_modelView[2][2] *= 0.5;

    // setup a hard-coded position
    if (gl_InstanceID == 0)
    {
        u_modelView[3][0] = -0.5;
        u_modelView[3][1] = -0.5;
    }
    else if (gl_InstanceID == 1)
    {
        u_modelView[3][0] = 0.5;
        u_modelView[3][1] = -0.5;
    }
    else if (gl_InstanceID == 2)
    {
        u_modelView[3][0] = 0.5;
        u_modelView[3][1] = 0.5;
    }
    else if (gl_InstanceID == 3)
    {
        u_modelView[3][0] = -0.5;
        u_modelView[3][1] = 0.5;
    }

    vec4 positionV4 = vec4(i_position, 1.0);
    gl_Position = u_projection * u_modelView * positionV4;
    m_texCoord = i_uv;
	m_normal = normalize(mat3(u_modelView) * i_normal);
	m_fragPos = vec3(u_modelView * positionV4);
}



#endif

#if 0
uniform mat4 u_projection;
uniform mat4 u_modelView;
// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// // Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// // Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragPos;

void main()
{
    vec4 positionV4 = vec4(i_position, 1.0);
    gl_Position = u_projection * u_modelView * positionV4;
    m_texCoord = i_uv;
	m_normal = normalize(mat3(u_modelView) * i_normal);
	m_fragPos = vec3(u_modelView * positionV4);
}
#endif

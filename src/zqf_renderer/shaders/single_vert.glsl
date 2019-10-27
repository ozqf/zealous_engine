#version 330 core

// Program data inputs
uniform mat4 u_projection;
uniform mat4 u_modelView;
uniform mat4 u_depthMVP;
//uniform mat4 u_model;

// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

// outputs to fragment shader
out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragViewPos;
out vec4 m_fraglightSpacePos;
// int outs must be 'flat', ie no smoothing function etc.
flat out int m_instanceID;

void main()
{
	vec4 positionV4 = vec4(i_position, 1.0);
   	gl_Position = u_projection * u_modelView * positionV4;
   	m_texCoord = i_uv;
	m_normal = normalize(mat3(u_modelView) * i_normal);
	m_fragViewPos = vec3(u_modelView * positionV4);
	m_instanceID = 1;

	m_fraglightSpacePos = u_depthMVP * positionV4;
}

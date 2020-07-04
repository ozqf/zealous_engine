#version 330 core

// Program data inputs
uniform mat4 u_projection;
uniform mat4 u_modelView;
uniform mat4 u_model;
//uniform mat4 u_view;
uniform int u_isBillboard;

// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

// outputs to fragment shader
out vec3 m_worldPos;
out vec2 m_texCoord;
out vec3 m_normal;

void main()
{
	vec4 positionV4 = vec4(i_position, 1.0);
	// outputs for gbuffer
   	m_texCoord = i_uv;
	// world space
	if (u_isBillboard == 1)
	{
		#if 0 // tell frag shader to face light source
		m_normal = vec3(0, 0, 0);
		#endif
		// old random attempts and doing 'proper' lighting on billboards.
		// doesn't work though
		#if 0
		//vec3 viewNormal = vec3(0, 0, 0);// -u_view[2].xyz;
		vec3 geoNormal = vec3(0, 0, 0);
		// meh - mix two methods - still crap but whatever.
		//vec3 viewNormal = -u_view[2].xyz;
		vec3 viewNormal = u_view[2].xyz;
		//vec3 geoNormal = mat3(u_model) * i_normal;
		//m_normal = normalize(viewNormal + geoNormal);
		m_normal = vec3(0, 0, 0);
		//m_normal = normalize(mat3(u_model) * i_normal);
		#endif
		#if 0
		vec3 toView = vec3(positionV4) - u_view[3].xyz;
		toView = normalize(toView);
		m_normal = toView;
		#endif
		#if 1 // standard 3D
		m_normal = normalize(mat3(u_model) * i_normal);
		#endif
		// reset rotation
		vec3 scale;
		mat4 mv = u_modelView;
		scale.x = length(mv[0].xyz);
		scale.y = length(mv[1].xyz);
		scale.z = length(mv[2].xyz);
		mv[0].xyz = vec3(1, 0, 0) * scale.x;
		mv[1].xyz = vec3(0, 1, 0) * scale.y;
		mv[2].xyz = vec3(0, 0, 1) * scale.z;
		gl_Position = u_projection * mv * positionV4;
	}
	else
	{
		m_normal = normalize(mat3(u_model) * i_normal);
		gl_Position = u_projection * u_modelView * positionV4;
	}
	
	m_worldPos = vec3(u_model * positionV4);
	// view space
	//m_normal = normalize(mat3(u_modelView) * i_normal);
	//m_worldPos = vec3(positionV4);
}

#version 330 core

// Program data inputs
uniform mat4 u_projection;
//uniform mat4 u_modelView;
//uniform mat4 u_model;

// Batch data info
uniform sampler2D u_dataTex;
// For calculating offsets into the data texture
uniform int u_pixelsPerBatchItem;
uniform int u_dataTexWidth;
// start of data for this batch
uniform int u_batchOffsetIndex;


/**
Note: Max number of attribs is 16!
*/
// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

// Vertex Attrib 3 (and 4, 5, 6!)
//layout (location = 3) in mat4 i_modelView;
//layout (location = 3) in vec4 i_column0;
//layout (location = 4) in vec4 i_column1;
//layout (location = 5) in vec4 i_column2;
//layout (location = 6) in vec4 i_column3;

// outputs to fragment shader
out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragPos;
flat out int m_instanceID;
//out float m_instanceColour;

void main()
{
	int dataIndex = gl_InstanceID * u_pixelsPerBatchItem;
	ivec2 pixel = ivec2(dataIndex, 0);
	
	mat4 mv;
	// From vbo batch data
	//mv[0] = i_column0;
	//mv[1] = i_column1;
	//mv[2] = i_column2;
	//mv[3] = i_column3;

	// From data texture
	mv[0] = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	mv[1] = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	mv[2] = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	mv[3] = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;


	vec4 positionV4 = vec4(i_position, 1.0);
	gl_Position = u_projection*mv*positionV4;
	
	// info for fragment shader
    m_texCoord = i_uv;
	m_normal = normalize(mat3(mv) * i_normal);
	m_fragPos = vec3(mv * positionV4);
	m_instanceID = gl_InstanceID;
}

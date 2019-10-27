#version 330 core

uniform sampler2D u_dataTex2D;

// Vertex Attrib 0
layout (location = 0) in vec3 i_position;
// Vertex Attrib 1
layout (location = 1) in vec2 i_uv;
// Vertex Attrib 2
layout (location = 2) in vec3 i_normal;

// outputs to fragment shader
out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragPos;
flat out int m_instanceID;
flat out int m_dataPixelIndex;


ivec2 DataIndexToPixel(int index, int imageWidth)
{
	return ivec2(index % imageWidth, int(index / imageWidth));
}

void main()
{
    int dataPixel = (u_pixelsPerBatchItem * gl_InstanceID) + u_batchOffsetIndex;
    mat4 mv;
    mv[0] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));
	mv[1] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));
	mv[2] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));
	mv[3] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));

	vec4 positionV4 = vec4(i_position, 1.0);
	gl_Position = u_projection*mv*positionV4;
	
	// info for fragment shader
    m_texCoord = i_uv;
	m_normal = normalize(mat3(mv) * i_normal);
	m_fragPos = vec3(mv * positionV4);
	m_instanceID = gl_InstanceID;
	m_dataPixelIndex = dataPixel;
}
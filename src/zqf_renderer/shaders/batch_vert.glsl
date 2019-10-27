#version 330 core

// Batch data info
uniform sampler2D u_diffuseTex;
uniform sampler2D u_lightmap;
uniform sampler2D u_dataTex2D;
uniform sampler1D u_dataTex1D;

// Program data inputs
uniform mat4 u_projection;
// model view is for debugging only!
// in batch form MV is pulled from batch data.
uniform mat4 u_modelView;
//uniform mat4 u_model;

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

// outputs to fragment shader
out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragPos;
flat out int m_instanceID;
flat out int m_dataPixelIndex;
//out float m_instanceColour;

ivec2 DataIndexToPixel(int index, int imageWidth)
{
	return ivec2(index % imageWidth, int(index / imageWidth));
}

void main()
{
	int dataPixel = (u_pixelsPerBatchItem * gl_InstanceID) + u_batchOffsetIndex;
	//ivec2 pixel = DataIndexToPixel(dataOrigin++, u_dataTexWidth);
	
	mat4 mv;
	//mv = u_modelView;
	#if 0 // From data texture single block
	mv[0] = vec4(texelFetch(u_dataTex2D, pixel, 0));
	pixel.x++;
	mv[1] = vec4(texelFetch(u_dataTex2D, pixel, 0));
	pixel.x++;
	mv[2] = vec4(texelFetch(u_dataTex2D, pixel, 0));
	pixel.x++;
	mv[3] = vec4(texelFetch(u_dataTex2D, pixel, 0));
	pixel.x++;
	#endif

	#if 1 // From data texture assuming row may change
	mv[0] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));
	mv[1] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));
	mv[2] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));
	mv[3] = vec4(texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0));
	#endif

	vec4 positionV4 = vec4(i_position, 1.0);
	gl_Position = u_projection*mv*positionV4;
	
	// info for fragment shader
    m_texCoord = i_uv;
	m_normal = normalize(mat3(mv) * i_normal);
	m_fragPos = vec3(mv * positionV4);
	m_instanceID = gl_InstanceID;
	m_dataPixelIndex = dataPixel;
}

#version 330

uniform mat4 u_projection;
uniform mat4 u_view;
// uniform int u_instanceCount;
uniform sampler2D u_dataTexture;
uniform int u_dataStride;
uniform int u_dataTexSize;

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;

out vec2 m_texCoord;
out vec3 m_normal;
out vec3 m_fragPos;
// flat - no interpolation
flat out int m_instanceID;
flat out vec4 m_colour;

ivec2 DataIndexToPixel(int index, int imageWidth)
{
	return ivec2(index % imageWidth, int(index / imageWidth));
}

vec4 ReadDataPixel(int dataItemNumber)
{
    int root = u_dataStride * m_instanceID;
    ivec2 pixel = DataIndexToPixel(root + dataItemNumber, u_dataTexSize);
    return texelFetch(u_dataTexture, pixel, 0);
}

void main()
{
    // pass instance to frag shader
    m_instanceID = gl_InstanceID;

    // read instance data
    vec4 data1 = ReadDataPixel(0);
    vec4 data2 = ReadDataPixel(1);
    
    mat4 u_modelView = u_projection;

    mat4 u_model;
    
    u_model[0][0] = 0.1; // scale x
    u_model[0][1] = 0;
    u_model[0][2] = 0;
    u_model[0][3] = 0;

    u_model[1][0] = 0;
    u_model[1][1] = 0.1; // scale y
    u_model[1][3] = 0;
    u_model[1][2] = 0;

    u_model[2][0] = 0;
    u_model[2][1] = 0;
    u_model[2][2] = 0.1; // scale z
    u_model[2][3] = 0;
    
    u_model[3][0] = data1.x; // pos x
    u_model[3][1] = data1.y; // pos y
    u_model[3][2] = data1.z; // pos z
    u_model[3][3] = 1;      // pos w

    
    
    u_modelView[0][0] *= 0.02f;
    u_modelView[1][1] *= 0.02f;
    u_modelView[2][2] *= 0.02f;

    // pos
    u_modelView[3][0] = data1.x;
    u_modelView[3][1] = data1.y;
    u_modelView[3][2] = data1.z;
    u_modelView[3][3] = 1;
    
    vec4 positionV4 = vec4(i_position, 1.0);
    // gl_Position = u_projection * u_modelView * positionV4;
    gl_Position = u_projection * u_model * u_view * positionV4;
    // m_texCoord = i_uv;
    // m_texCoord = vec2(0.25, 0.25);
    m_texCoord.x = data2.x + ((data2.z - data2.x) * i_uv.x);
    m_texCoord.y = data2.y + ((data2.w - data2.y) * i_uv.y);
	m_normal = normalize(mat3(u_modelView) * i_normal);
	m_fragPos = vec3(u_modelView * positionV4);
}

#version 330

#define DEG2RAD 0.0174532925

uniform mat4 u_projection;
uniform mat4 u_view;
// uniform int u_instanceCount;
uniform sampler2D u_dataTexture;
uniform int u_dataStride;
uniform int u_dataTexSize;
uniform int u_isBillboard;

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

mat4 CreateScaleM4x4(float x, float y, float z)
{
    mat4 m;
    m[0][0] = x; // scale x
    m[0][1] = 0;
    m[0][2] = 0;
    m[0][3] = 0;

    m[1][0] = 0;
    m[1][1] = y; // scale y
    m[1][3] = 0;
    m[1][2] = 0;

    m[2][0] = 0;
    m[2][1] = 0;
    m[2][2] = z; // scale z
    m[2][3] = 0;
    
    m[3][0] = 0;            // pos x
    m[3][1] = 0;            // pos y
    m[3][2] = 0;            // pos z
    m[3][3] = 1;            // pos w
    return m;
}

mat4 CreateRotateM4x4(float radians)
{
    mat4 rot;
    rot[0][0] = cos(radians);
    rot[0][1] = sin(radians);
    rot[0][2] = 0;
    rot[0][3] = 0;

    rot[1][0] = -sin(radians);
    rot[1][1] = cos(radians);
    rot[1][2] = 0;
    rot[1][3] = 0;

    rot[2][0] = 0;
    rot[2][1] = 0;
    rot[2][2] = 1;
    rot[2][3] = 0;

    rot[3][0] = 0;
    rot[3][1] = 0;
    rot[3][2] = 0;
    rot[3][3] = 1;
    return rot;
}

vec3 RotateSpritVertex(vec3 pos, float radians)
{
    mat3 rot;
    rot[0][0] = cos(radians);
    rot[0][1] = sin(radians);
    rot[0][2] = 0;

    rot[1][0] = -sin(radians);
    rot[1][1] = cos(radians);
    rot[1][2] = 0;

    rot[2][0] = 0;
    rot[2][1] = 0;
    rot[2][2] = 1;
    return pos * rot;
}

void main()
{
    // pass instance to frag shader
    m_instanceID = gl_InstanceID;

    // read instance data
    vec4 data1 = ReadDataPixel(0);
    vec4 data2 = ReadDataPixel(1);
    vec4 data3 = ReadDataPixel(2);
    
    mat4 u_model;
    /*
    u_model[0][0] = data3.x; // scale x
    u_model[0][1] = 0;
    u_model[0][2] = 0;
    u_model[0][3] = 0;

    u_model[1][0] = 0;
    u_model[1][1] = data3.y; // scale y
    u_model[1][3] = 0;
    u_model[1][2] = 0;

    u_model[2][0] = 0;
    u_model[2][1] = 0;
    u_model[2][2] = 1; // scale z
    u_model[2][3] = 0;
    
    u_model[3][0] = data1.x;    // pos x
    u_model[3][1] = data1.y;    // pos y
    u_model[3][2] = data1.z;    // pos z
    u_model[3][3] = 1;          // pos w
    */
    
    vec4 positionV4 = vec4(i_position, 1.0);
    // apply scale then rotate in model space
    mat4 scale = CreateScaleM4x4(data3.x, data3.y, 1);
    mat4 rot = CreateRotateM4x4(data1.w);
    positionV4 *= scale;
    positionV4 *= rot;

    u_model[0][0] = 1; // scale x
    u_model[0][1] = 0;
    u_model[0][2] = 0;
    u_model[0][3] = 0;

    u_model[1][0] = 0;
    u_model[1][1] = 1; // scale y
    u_model[1][3] = 0;
    u_model[1][2] = 0;

    u_model[2][0] = 0;
    u_model[2][1] = 0;
    u_model[2][2] = 1; // scale z
    u_model[2][3] = 0;
    
    u_model[3][0] = data1.x;    // pos x
    u_model[3][1] = data1.y;    // pos y
    u_model[3][2] = data1.z;    // pos z
    u_model[3][3] = 1;          // pos w
    
    mat4 u_modelView = u_view * u_model;
    
    // vec4 positionV4 = vec4(RotateSpritVertex(i_position, 45 * DEG2RAD), 1.0);
    // vec4 positionV4 = vec4(i_position, 1.0);
    if (u_isBillboard == 0)
    {
        // regular 3d
        gl_Position = u_projection * u_modelView * positionV4;
    }
    else
    {
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
    
    m_texCoord.x = data2.x + ((data2.z - data2.x) * i_uv.x);
    m_texCoord.y = data2.y + ((data2.w - data2.y) * i_uv.y);
	m_normal = normalize(mat3(u_modelView) * i_normal);
	m_fragPos = vec3(u_modelView * positionV4);
}

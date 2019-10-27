#version 330 core

// input from vert shader
in vec2 m_texCoord;
in vec3 m_fragPos;
in vec3 m_normal;
// flat - must match signature of output from vert shader
flat in int m_instanceID;
flat in ivec2 m_dataPixel;
flat in int m_dataPixelIndex;
//int m_instanceColour;

// final output colour
out vec4 outputColor;

// Program data inputs
uniform sampler2D u_diffuseTex;
uniform sampler2D u_lightmap;
uniform sampler2D u_dataTex2D;
uniform sampler1D u_dataTex1D;

// Number of lights for each instance
uniform int u_numLights;
// number of pixels in data buffer used by each light
uniform int u_lightDataStride;
uniform int u_dataTexWidth;

uniform int u_debugMode;

ivec2 DataIndexToPixel(int index, int imageWidth)
{
	return ivec2(index % imageWidth, int(index / imageWidth));
}

// Infinite distance directional light
float CalcLightDirectionDiff(vec3 dir, vec3 colour, vec3 fragNormal)
{
	// apply sunlight only
	dir.z = -dir.z;
	dir.x = -dir.x;
	dir.y = -dir.y;
	return max(dot(fragNormal, dir), 0.0);
}

// Infinite distance directional light
vec4 CalcDirLight(vec3 dir, vec3 colour, vec3 fragNormal)
{
	// apply sunlight only
	dir.z = -dir.z;
	dir.x = -dir.x;
	dir.y = -dir.y;
	float diff = max(dot(fragNormal, dir), 0.0);
	return vec4(diff * colour, 1);
}

vec4 CalcPointLight(vec3 lightPos, vec3 colour, vec4 lightSettings, vec3 fragPos, vec3 fragNormal)
{
	vec3 dir = normalize(lightPos - fragPos);
	float diff = max(dot(fragNormal, dir), 0.0);
	float dist = distance(fragPos, lightPos);
	//float maxRange = 50;
	float maxRange = lightSettings.x;
	float scalar = 1 - (dist / maxRange);
	return vec4(diff * (colour * scalar), 1);
}

void main()
{
	int dataPixel = m_dataPixelIndex;
	#if 0
	outputColor = texture2D(u_diffuseTex, m_texCoord);
	#endif

	#if 1
	// read from data.
	// 1 pixel for ambient, 3 pixels per light
	//ivec2 cursor = m_dataPixel;
	vec4 diffuse = texture2D(u_diffuseTex, m_texCoord);
	vec4 ambient = texelFetch(u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0);
	//cursor.x++;
	
	outputColor = diffuse * ambient;
	
	for (int i = 0; i < u_numLights; ++i)
	{
		// Pull data
		vec4 pointPosition = texelFetch(
			u_dataTex2D, DataIndexToPixel(dataPixel++, u_dataTexWidth), 0);
		vec4 pointColour = texelFetch(u_dataTex2D,
			DataIndexToPixel(dataPixel++, u_dataTexWidth), 0);
		vec4 settings = texelFetch(u_dataTex2D,
			DataIndexToPixel(dataPixel++, u_dataTexWidth), 0);
		
		// Calc
		vec4 pointResult = CalcPointLight(
			vec3(pointPosition),
			vec3(pointColour),
			settings,
			m_fragPos,
			m_normal);
		outputColor += (pointResult * diffuse);
	}
	
	#endif
	// Debug
	#if 0
	outputColor = vec4(0.5, 0.5, 0.5, 1);
	#endif
}

#version 330 core

#define MAX_POINT_LIGHTS 2

// input from vert shader
in vec2 m_texCoord;
in vec3 m_fragPos;
in vec3 m_normal;
// flat - must match signature of output from vert shader
flat in int m_instanceID;
//int m_instanceColour;

// final output colour
out vec4 outputColor;

// Program data inputs
uniform sampler2D u_diffuseTex;
uniform sampler2D u_lightmap;
uniform sampler2D u_dataTex;

// Number of lights for each instance
uniform int u_numLights;
// number of pixels in data buffer used by each light
uniform int u_lightDataStride;

uniform int u_debugMode;

// Infinite distance directional light
float CalcLightDirectionDiff(
	vec3 dir,
	vec3 colour,
	vec3 fragNormal)
{
	// apply sunlight only
	dir.z = -dir.z;
	dir.x = -dir.x;
	dir.y = -dir.y;
	
	return max(dot(fragNormal, dir), 0.0);
}


// Infinite distance directional light
vec4 CalcDirLight(
	vec3 dir,
	vec3 colour,
	vec3 fragNormal)
{
	// apply sunlight only
	dir.z = -dir.z;
	dir.x = -dir.x;
	dir.y = -dir.y;
	
	float diff = max(dot(fragNormal, dir), 0.0);
	return vec4(diff * colour, 1);
}

vec4 CalcPointLight(vec3 lightPos, vec3 colour, vec3 fragPos, vec3 fragNormal)
{
	vec3 dir = normalize(lightPos - fragPos);
	float diff = max(dot(fragNormal, dir), 0.0);
	float dist = distance(fragPos, lightPos);
	float maxRange = 30;
	float scalar = 1 - (dist / maxRange);
	return vec4(diff * (colour * scalar), 1);
}

void main()
{
	outputColor = texture2D(u_diffuseTex, m_texCoord);
	
	#if 0
	// Extract lighting data from data texture
	int numPixelsPerLight = u_lightDataStride;
	int numLightsPerObject = u_numLights;
	int index = (m_instanceID) * (numPixelsPerLight * numLightsPerObject);
	ivec2 pixel = ivec2(index, 0);
	
	// Ambient light, first pixel
	//vec4 ambientColour = vec4(texelFetch(u_dataTex, pixel, 0));
	//pixel.x++;

	vec4 lightPos = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	vec4 lightColour = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	vec4 lightPos2 = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	vec4 lightColour2 = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	vec4 lightPos3 = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	vec4 lightColour3 = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	vec4 lightPos4 = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	vec4 lightColour4 = vec4(texelFetch(u_dataTex, pixel, 0));
	pixel.x++;
	#if 0
	outputColor = vec4(lightColour);
	outputColor = vec4(lightPos);
	#endif
	#if 1
	// Diffuse
	vec4 diffuseTexColour = texture2D(u_diffuseTex, m_texCoord);
	
	// calculate lights
	vec4 colour = CalcPointLight(
		vec3(lightPos), vec3(lightColour), m_fragPos, m_normal);
	colour = (colour * diffuseTexColour);
	
	vec4 colour2 = CalcPointLight(
		vec3(lightPos2), vec3(lightColour2), m_fragPos, m_normal);
	colour2 = (colour2 * diffuseTexColour);
	
	vec4 colour3 = CalcPointLight(
		vec3(lightPos3), vec3(lightColour3), m_fragPos, m_normal);
	colour3 = (colour3 * diffuseTexColour);
	
	vec4 colour4 = CalcPointLight(
		vec3(lightPos4), vec3(lightColour4), m_fragPos, m_normal);
	colour4 = (colour4 * diffuseTexColour);
	
	// add lights
	outputColor = vec4(0, 0, 0, 1);
	//outputColor += ambientColour;
	outputColor += colour;
	outputColor += colour2;
	outputColor += colour3;
	outputColor += colour4;
	#endif

	#if 0
	outputColor = texture2D(u_diffuseTex, m_texCoord);
	#endif
	
	#endif
}

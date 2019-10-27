#version 330 core

#define MAX_POINT_LIGHTS 2

// input from vert shader
in vec2 m_texCoord;
in vec3 m_fragViewPos;
in vec3 m_normal;

in vec4 m_fraglightSpacePos;
// flat - must match signature of output from vert shader
flat in int m_instanceID;
//int m_instanceColour;

// final output colour
out vec4 outputColor;

// Program data inputs
uniform sampler2D u_diffuseTex;
//uniform sampler2D u_lightmap;
uniform sampler1D u_dataTex;

// Lighting
uniform sampler2D u_shadowMap;
uniform vec3 u_lightViewPos;
//uniform vec3 u_lightDir;
uniform vec3 u_lightColour;

uniform int u_debugMode;

uniform float u_shadowBias;

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
	return vec4(diff * colour, 1);
}

float CalcPointLightFactor(vec3 lightPos, vec3 fragPos, vec3 fragNormal)
{
	vec3 dir = normalize(lightPos - fragPos);
	float diff = max(dot(fragNormal, dir), 0.0);
	return diff;
}

float CalcDirectLightFactor(vec3 dir, vec3 fragPos, vec3 fragNormal)
{
	float diff = max(dot(fragNormal, dir), 0.0);
	return diff;
}

float CalcShadowFactorNoFiltering(vec4 lightSpacePos)
{
	float minBrightness = 0.2;
	// Q: why does tutorial have this / lightSpacePos.w ?
	// A: required if projection is perspective not orthographic
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	vec2 uvCoords;
	uvCoords.x = 0.5 * projCoords.x + 0.5;
	uvCoords.y = 0.5 * projCoords.y + 0.5;
	float z = 0.5 * projCoords.z + 0.5;
	//if (uvCoords.x < 0 || uvCoords.x > 1 || uvCoords.y < 0 || uvCoords.y > 1)
	//{
	//	return minBrightness;
	//}

	// Note: If shadow acne occurs, increase this.
	// TODO: Dynamically adjusted bias
	//float bias = 0.005;

	float depth = texture(u_shadowMap, uvCoords).x;
	//float depth = textureProj(u_shadowMap, projCoords, bias).x;
	if (depth < (z - u_shadowBias))
	{
		return minBrightness;
	}
	return 1;
}

float CalcShadowFactor(vec4 lightSpacePos)
{
	float minBrightness = 0.2;
	// Q: why does tutorial have this / lightSpacePos.w ?
	// A: required if projection is perspective not orthographic
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	vec2 uvCoords;
	uvCoords.x = 0.5 * projCoords.x + 0.5;
	uvCoords.y = 0.5 * projCoords.y + 0.5;
	float z = 0.5 * projCoords.z + 0.5;

	float shadow = 0;
	vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			vec2 sampleCoords = uvCoords.xy + vec2(x, y);
			float pcfDepth = texture(u_shadowMap, sampleCoords).x;
			if (pcfDepth > (z - u_shadowBias))
			{ shadow += 1.0; }
		}
	}
	return shadow / 9.0;
	/*float depth = texture(u_shadowMap, uvCoords).x;
	if (depth < (z - u_shadowBias))
	{
		return minBrightness;
	}
	return 1;
	*/
}

void main()
{
	int debugMode = 1;

	switch (debugMode)
	{
		default:
		float shadowFactor = CalcShadowFactor(m_fraglightSpacePos);
		vec4 diffuseTexColour = texture2D(u_diffuseTex, m_texCoord);
		float lightFactor = CalcPointLightFactor(
			u_lightViewPos, m_fragViewPos, m_normal);
		float brightness;
		if (shadowFactor < lightFactor)
		{ brightness = shadowFactor; }
		else
		{ brightness = lightFactor; }

		outputColor = vec4(diffuseTexColour * brightness);

		

		#if 0 // This version doesn't work. Why not?
		float fragDepth = m_fraglightSpacePos.z;
		vec4 shadowPixel = texture(u_shadowMap, m_fraglightSpacePos.xy);
		//outputColor = vec4(m_fraglightSpacePos.x, m_fraglightSpacePos.y, 0, 1);
		// Read red channel as shadow is a depth only texture and has
		// only one channel
		float shadowDepth = shadowPixel.r;
		float brightness = 1;
		if (shadowDepth < fragDepth)
		{
			brightness = 0.2;
			outputColor = vec4(0.2, 0.2, 0.2, 0.2);
		}
		vec4 diffuseTexColour = texture2D(u_diffuseTex, m_texCoord);
		outputColor = vec4(
			diffuseTexColour.x * brightness,
			diffuseTexColour.y * brightness,
			diffuseTexColour.z * brightness,
			1);
		#endif

		break;
	}
	#if 0 // TODO: Delete this cack when code copied out
	//switch(u_debugMode)
	switch(debugMode)
	{
		case 2:
		{
		// apply sunlight
		vec4 diffuseTexColour = texture2D(u_diffuseTex, m_texCoord);
		vec4 sunLight = CalcDirLight(u_sunDir, u_sunColour, m_normal);
		outputColor = vec4(sunLight * diffuseTexColour);
		outputColor.x = (outputColor.x * 0.9) + 0.1;
		outputColor.y = (outputColor.y * 0.9) + 0.1;
		outputColor.z = (outputColor.z * 0.9) + 0.1;
		}
		break;
		case 3:
		// Sunlight B&W
		float diff = CalcLightDirectionDiff(u_sunDir, u_sunColour, m_normal);
		outputColor = vec4(diff, diff, diff, 1);
		break;

		case 4:
		{
		
		// apply ambience only
		float ambientStrength = 0.5;
		vec4 ambient = vec4(ambientStrength * u_ambientColour, 1);
		vec4 objectColour = texture2D(u_diffuseTex, m_texCoord);
		outputColor = vec4(ambient * objectColour);
		}
		break;

		case 5:
		{
		vec3 dir = normalize(u_pointLightPos - m_fragWorldPos);
		outputColor = vec4(dir, 1);
		outputColor.x += 1;
		outputColor.y += 1;
		outputColor.z += 1;
		outputColor.x *= 0.5;
		outputColor.y *= 0.5;
		outputColor.z *= 0.5;
		}
		break;

		case 6:
		{
		vec3 dir = normalize(u_pointLightPos - m_fragWorldPos);
		float diff = max(dot(m_normal, dir), 0.0);
		outputColor = vec4(diff, diff, diff, 1);
		}
		break;

		case 8:
		// Show raw normal as colour
		outputColor = vec4(m_normal, 1);
		outputColor.x += 1;
		outputColor.y += 1;
		outputColor.z += 1;
		outputColor.x *= 0.5;
		outputColor.y *= 0.5;
		outputColor.z *= 0.5;
		break;
		// not implemented
		case 9:
		// mix two textures
		outputColor = mix(texture2D(u_diffuseTex, m_texCoord), texture2D(u_lightmap, m_texCoord), 0.2);
		break;
		
		// fallback - just output texture
		default:
		
		// Show instance number as range white (0, first) to black (1, last):
		float instanceLevel = float(m_instanceID) / float(100);
		instanceLevel = 1 - instanceLevel;
		outputColor = vec4(instanceLevel, instanceLevel, instanceLevel, 1);
		

		/*
		// Read a random pixel
		int texLOD = 0;
		outputColor = texelFetch(u_diffuseTex, ivec2(0, 0), texLOD);
		*/
		
		//outputColor = vec4(1, 0, 1, 1);
		//outputColor = texture2D(u_diffuseTex, m_texCoord);
		
		//outputColor = texelFetch(u_dataTex, 0, 0);
		
		break;
	}
	#endif
}

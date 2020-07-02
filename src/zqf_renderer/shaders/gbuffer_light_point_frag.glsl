#version 330

// view transforms
uniform mat4 u_worldProjection;
uniform mat4 u_worldView;

// gBuffer
uniform sampler2D u_positionTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_colourTex;
uniform sampler2D u_emissionTex;

// light params
uniform vec3 u_lightWorldPos;
uniform vec3 u_lightWorldDir;
uniform vec3 u_lightColour;
uniform float u_lightMultiplier;
uniform float u_lightRange;

in vec2 m_texCoord;
out vec4 outputColor;

vec4 CalcPointLight(vec3 lightPos, vec3 colour, float lightRange, vec3 fragPos, vec3 fragNormal)
{
	vec3 dir = normalize(lightPos - fragPos);
	float diff = max(dot(fragNormal, dir), 0.0);
	float dist = distance(fragPos, lightPos);
	//float maxRange = 50;
	float maxRange = lightRange;
	float scalar = 1 - (dist / maxRange);
	return vec4(diff * (colour * scalar), 1);
}

// Oh... don't have the depth value for
// the original pixel here yet...
// Try this?
// https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
// - need to pass specific distance, not depth!
vec3 DepthToWorldPos()
{
	return vec3(0, 0, 0);
}

void main()
{
    // gather gbuffer info
    vec4 colour = vec4(texture2D(u_colourTex, m_texCoord));
	float depth = colour.w;
	colour.w = 1;
    vec3 normal = vec3(texture2D(u_normalTex, m_texCoord));
    vec3 fragWorldPos = vec3(texture2D(u_positionTex, m_texCoord));
	vec4 emission = vec4(texture2D(u_emissionTex, m_texCoord));
	#if 1
    vec4 lightResult = CalcPointLight(
    	u_lightWorldPos, u_lightColour, u_lightRange, fragWorldPos, normal);
    lightResult *= u_lightMultiplier;
    outputColor = colour * lightResult;
    #endif
	#if 0 // read depth - only works with one full screen quad
	outputColor = vec4(depth, depth, depth, 1);
	#endif
}
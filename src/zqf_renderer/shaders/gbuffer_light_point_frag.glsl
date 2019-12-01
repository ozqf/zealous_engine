#version 330

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

void main()
{
    // gather gbuffer info
    vec3 colour = vec3(texture2D(u_colourTex, m_texCoord));
    vec3 normal = vec3(texture2D(u_normalTex, m_texCoord));
    vec3 fragWorldPos = vec3(texture2D(u_positionTex, m_texCoord));
	vec4 emission = vec4(texture2D(u_emissionTex, m_texCoord));
	
    float lightMul = emission.x;
    if (lightMul > 0.9)
    {
        outputColor = vec4(colour, 1);
    }
    else
    {
        vec4 lightResult = CalcPointLight(
        u_lightWorldPos, u_lightColour, u_lightRange, fragWorldPos, normal);
    
        lightResult *= u_lightMultiplier;
        outputColor = vec4(colour, 1) * lightResult;
    }
    
}
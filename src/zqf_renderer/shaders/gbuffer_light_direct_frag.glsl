#version 330

// gBuffer
uniform sampler2D u_positionTex;
uniform sampler2D u_normalTex;
uniform sampler2D u_colourTex;

// light params
uniform vec3 u_lightWorldPos;
uniform vec3 u_lightWorldDir;
uniform vec3 u_lightColour;
uniform float u_lightMultiplier;
uniform float u_lightRange;

in vec2 m_texCoord;
out vec4 outputColor;

float CalcDirectLightFactor(vec3 dir, vec3 fragPos, vec3 fragNormal)
{
	float diff = max(dot(fragNormal, dir), 0.0);
	return diff;
}

void main()
{
    // gather gbuffer info
    vec3 colour = vec3(texture2D(u_colourTex, m_texCoord));
    vec3 normal = vec3(texture2D(u_normalTex, m_texCoord));
    vec3 fragWorldPos = vec3(texture2D(u_positionTex, m_texCoord));

    float dirLightFactor = CalcDirectLightFactor(u_lightWorldDir, fragWorldPos, normal);
    dirLightFactor *= u_lightMultiplier;
    vec4 lightResult;
    lightResult.x = u_lightColour.x * dirLightFactor;
    lightResult.y = u_lightColour.y * dirLightFactor;
    lightResult.z = u_lightColour.z * dirLightFactor;
    lightResult.w = 1;
    outputColor = vec4(vec4(colour, 1) * lightResult);
}
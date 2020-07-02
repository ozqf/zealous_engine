#version 330

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColour;
layout (location = 3) out vec4 gEmission;

uniform mat4 u_modelView;
uniform sampler2D u_colourTex;
uniform sampler2D u_emissionTex;

in vec3 m_worldPos;
in vec2 m_texCoord;
in vec3 m_normal;

void main()
{
	//extract
	float depth = gl_FragCoord.z;
	//depth = 1 * depth;
    vec4 colour = texture2D(u_colourTex, m_texCoord);
	colour.w = depth;
    vec4 emission = texture2D(u_emissionTex, m_texCoord);
	// write
    if (colour.w > 0.5)
    {
        gPosition = m_worldPos;
        #if 0
        if (u_isBillboard == 1)
        {
            gNormal = -u_modelView[2].xyz;
        }
        else
        {
            gNormal = m_normal;
        }
        #endif
        #if 1
        gNormal = m_normal;
        #endif
        gColour = colour;
        gEmission = emission;
        //gEmission = vec4(0, 0, 0, 1);
    }
    else
    {
        discard;
    }
}
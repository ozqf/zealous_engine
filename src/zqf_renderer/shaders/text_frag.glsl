#version 330

uniform sampler2D u_diffuseTex;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

out vec4 outputColor;

void main()
{
    vec4 colour = texture2D(u_diffuseTex, m_texCoord);

    if (colour.w > 0.5)
    {
        outputColor = texture2D(u_diffuseTex, m_texCoord);
    }
    else
    {
        // black - makes text readable against any background
        outputColor = vec4(0, 0, 0, 1);

        // TODO: Research - discard is expensive/bad (unnecessary frag shader call)?
        //discard;
    }
}
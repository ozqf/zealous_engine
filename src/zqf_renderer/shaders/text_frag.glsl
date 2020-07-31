#version 330

uniform sampler2D u_diffuseTex;
uniform sampler2D u_stencilTex;
uniform vec4 u_colour;
uniform vec4 u_backgroundColour;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

out vec4 outputColor;

void main()
{
    float stencilRed = texture2D(u_stencilTex, m_texCoord).r;
    if (stencilRed > 0.5)
    {
        outputColor = u_colour;
    }
    else
    {
        if (u_backgroundColour.w < 0.5) { discard; }
        else { outputColor = u_backgroundColour; }
    }
    #if 0
    vec4 colour = texture2D(u_diffuseTex, m_texCoord);
    float stencilRed = texture2D(u_stencilTex, m_texCoord).r;

    if (colour.w > 0.5)
    {
        // if over stencil, override colour, otherwise the pixel is part of the
        // background.
        if (stencilRed > 0.5)
        {
            outputColor = mix(colour, u_colour, 0.5);
        }
        else
        {
            outputColor = colour;
        }
    }
    else
    {
        // black - makes text readable against any background
        //outputColor = vec4(0, 0, 0, 1);

        // TODO: Research - discard is expensive/bad (unnecessary frag shader call)?
        discard;
    }
    #endif
}
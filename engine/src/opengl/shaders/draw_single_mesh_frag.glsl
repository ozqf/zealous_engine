#version 330

uniform vec4 u_colour;
uniform sampler2D u_diffuseTex;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

out vec4 outputColor;

void main()
{
#if 1
   outputColor = vec4(1, 1, 1, 1);
#endif
#if 0
   // outputColor = u_colour;
#endif
#if 0 // Output depth
   float depthValue = gl_FragCoord.z;
   outputColor = vec4(u_colour.x * depthValue, u_colour.y * depthValue, u_colour.z * depthValue, 1);
#endif
#if 0 // output texture
   vec4 diffuse = texture2D(u_diffuseTex, m_texCoord) * u_colour;
   if (diffuse.w < 0.5) { discard; }
   outputColor = diffuse;
#endif
}
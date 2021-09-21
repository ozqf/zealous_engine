#version 330

uniform vec4 u_colour;
uniform sampler2D u_diffuseTex;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

out vec4 outputColor;

void main()
{
   #if 0
   if ((int(gl_FragCoord.x) % 2) == 0 && (int(gl_FragCoord.y) % 2) == 0)
   {
      discard;
   }
   outputColor = u_colour;
   #endif

   #if 1
   // move frag screen x/y into 0-1 range
   float x = (m_fragPos.x + 1) / 2;
   float y = (m_fragPos.y + 1) / 2;
   if (x < 0.5f)
   {
      if (y < 0.5f)
      {
         outputColor = vec4(1, 0, 0, 0.5);
      }
      else
      {
         outputColor = vec4(0, 1, 0, 0.5);
      }
   }
   else
   {
      if (y < 0.5f)
      {
         outputColor = vec4(1, 1, 0, 0.5);
      }
      else
      {
         // outputColor = vec4(1, 0, 1, 0.5);
         discard;
      }
   }
   #endif

   #if 0
   float x = m_fragPos.x;
   float y = m_fragPos.y;
   vec4 colour = u_colour;
   colour.x *= x;
   colour.y *= y;
   outputColor = colour;
   #endif
}

#version 330

uniform vec4 u_colour;
// uniform sampler2D u_diffuseTex;
uniform sampler2D u_tileDataTex;

uniform int u_tilesWide;
uniform int u_tilesHigh;

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

   #if 0
   vec4 data = texelFetch(u_tileDataTex, ivec2(0, 0), 0);
   outputColor = data;
   // outputColor = vec4(0.5, 0.5, 0.5, 0.5);
   #endif
   
   #if 1
   // move frag screen x/y into 0-1 range
   float x = (m_fragPos.x + 1) / 2;
   float y = (m_fragPos.y + 1) / 2;
   // calculate grid x/y
   // int tilesWide = 2;
   int gridX = int(x * u_tilesWide);
   int gridY = int(y * u_tilesHigh);
   // convert grid x/y to linear index
   int i = gridX + (gridY * u_tilesWide);
   vec4 data = texelFetch(u_tileDataTex, ivec2(0, i), 0);
   outputColor = data;
   #endif
}

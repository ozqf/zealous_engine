#version 330

uniform vec4 u_colour;
// uniform sampler2D u_diffuseTex;
uniform usampler2D u_tileDataTex;
uniform sampler2D u_lightsArrayTex;

uniform int u_tilesWide;
uniform int u_tilesHigh;

in vec2 m_texCoord;
in vec3 m_normal;
in vec3 m_fragPos;

out vec4 outputColor;

int FragPositionToTileIndex(vec3 fragPos, int tilesWide, int tilesHigh)
{
	// move frag screen x/y into 0-1 range
	float x = (fragPos.x + 1) / 2;
	float y = (fragPos.y + 1) / 2;
	// calculate grid x/y
	int gridX = int(x * tilesWide);
	int gridY = int(y * tilesHigh);
	// convert grid x/y to linear index
	int tileIndex = gridX + (gridY * tilesWide);
	return tileIndex;
}

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
	/*
	// move frag screen x/y into 0-1 range
	float x = (m_fragPos.x + 1) / 2;
	float y = (m_fragPos.y + 1) / 2;
	// calculate grid x/y
	int gridX = int(x * u_tilesWide);
	int gridY = int(y * u_tilesHigh);
	// convert grid x/y to linear index
	int tileIndex = gridX + (gridY * u_tilesWide);
	*/
	int tileIndex = FragPositionToTileIndex(m_fragPos, u_tilesWide, u_tilesHigh);
	// fetch first data row - count of light indices
	vec4 data = texelFetch(u_tileDataTex, ivec2(0, tileIndex), 0);
	// texture is single channel so only red is set
	int count = int(data.x);
	
	vec4 result = vec4(0, 0, 0, 0);
	for (int i = 0; i < count; ++i)
	{
		// get light index
		vec4 tileLightData = texelFetch(u_tileDataTex, ivec2(i + 1, tileIndex), 0);
		int lightIndex = int(tileLightData.x);
		// retrieve light data
		vec4 lightData1 = texelFetch(u_lightsArrayTex, ivec2(0, lightIndex), 0);
		vec4 lightData2 = texelFetch(u_lightsArrayTex, ivec2(1, lightIndex), 0);
		vec3 lightPos = vec3(lightData1);
		float radius = lightData1.w;
		vec3 rgb = vec3(lightData2);
		float strength = lightData2.w;
		result += vec4(rgb, 1);
		// outputColor = vec4(rgb, 0.5);
		// apply
	}
	outputColor = result;
	/*
	if (count == 1)
	{
		outputColor = vec4(1, 0, 0, 0.5);
	}
	else if (count == 2)
	{
		outputColor = vec4(0, 1, 0, 0.5);
	}
	else if (count == 3)
	{
		outputColor = vec4(0, 0, 1, 0.5);
	}
	else if (count == 4)
	{
		outputColor = vec4(1, 1, 0, 0.5);
	}
	else if (count == 255)
	{
		outputColor = vec4(0, 1, 1, 0.5);
	}
	// oops
	else if (count == 0)
	{
		outputColor = vec4(1, 0, 1, 0.2);
	}
	else
	{
		outputColor = vec4(1, 1, 1, 0.2);
	}
	*/
	// data.y = 0;
	// data.z = 0;
	// data.w = 0;
	// outputColor = data;
	#endif
}

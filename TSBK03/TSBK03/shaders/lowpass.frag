#version 150

in vec2 out_TexCoord;

out vec4 out_Color;

uniform sampler2D texUnit;
//uniform float t;

void main(void)
{
	vec3 acc = vec3(0,0,0);
	// --------------------------------------------------------------------------
	// Nedan bör ses över. Fungerar inte på AMD-GPUs, och vi borde
	// implementera ett vettigare filter (typ ett Gaussiskt).
	// Side = 1 ger 3x3 filter.
	// Side	= 2 ger 5x5 filter.
	// Side = n ger (2 * n + 1)x(2 * n + 1) filter.
	int side = 3;
	int area = (side * 2 + 1) * (side * 2 + 1);
	for(int x = -side; x <= side; x++)
	{
		for(int y = -side; y <= side; y++)
		{
			acc += vec3(textureOffset(texUnit, out_TexCoord, ivec2(x, y)));
		}
	}
	// --------------------------------------------------------------------------
	acc = acc / area;

    out_Color = vec4(acc, 1.0);
}
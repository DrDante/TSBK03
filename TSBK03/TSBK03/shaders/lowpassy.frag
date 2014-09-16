#version 150

in vec2 out_TexCoord;

out vec4 out_Color;

uniform sampler2D texUnit;

float gaussy[5] = float[](1, 4, 6, 4, 1);
int normFactor = 16;

void main(void)
{
	vec3 acc = vec3(0,0,0);
	
	acc += gaussy[0] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(0, -2)));
	acc += gaussy[1] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(0, -1)));
	acc += gaussy[2] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(0, 0)));
	acc += gaussy[3] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(0, 1)));
	acc += gaussy[4] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(0, 2)));
	acc /= normFactor;

    out_Color = vec4(acc, 1.0);
}
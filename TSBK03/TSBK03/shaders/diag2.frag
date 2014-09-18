#version 150

in vec2 out_TexCoord;

out vec4 out_Color;

uniform sampler2D texUnit;

float coeff[5] = float[](1, 1, 1, 1, 1);
int normFactor = 5;

void main(void)
{
	vec3 acc = vec3(0,0,0);

	acc += coeff[0] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(-2, 2)));
	acc += coeff[1] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(-1, 1)));
	acc += coeff[2] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(0, 0)));
	acc += coeff[3] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(1, -1)));
	acc += coeff[4] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(2, -2)));
	acc /= normFactor;

    out_Color = vec4(acc, 1.0);
}

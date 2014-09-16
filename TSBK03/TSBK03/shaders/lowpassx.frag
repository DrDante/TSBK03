#version 150

in vec2 out_TexCoord;

out vec4 out_Color;

uniform sampler2D texUnit;

float gaussx[5] = float[](1, 4, 6, 4, 1);
int normFactor = 16;

void main(void)
{
	vec3 acc = vec3(0,0,0);

	acc += gaussx[0] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(-2, 0)));
	acc += gaussx[1] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(-1, 0)));
	acc += gaussx[2] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(0, 0)));
	acc += gaussx[3] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(1, 0)));
	acc += gaussx[4] * vec3(textureOffset(texUnit, out_TexCoord, ivec2(2, 0)));
	acc /= normFactor;

    out_Color = vec4(acc, 1.0);
}
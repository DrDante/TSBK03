
#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

uniform float t;

void main(void)
{
	vec3 acc = vec3(0,0,0);

	/* Side = 1 ger 3x3 filter */
	/* 	= 2 ger 5x5 filter... */
	int side = 3;
	int area = (side*2+1)*(side*2+1);
	for(int x = -side; x <= side; x++){
		for(int y = -side; y <= side; y++){
			acc += vec3(textureOffset(texUnit, outTexCoord, ivec2(x, y)));
		}
	}
	acc = acc / area;

    out_Color = vec4(acc, 1.0);
}

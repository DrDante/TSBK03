#version 150

out vec4 out_color;
in vec4 lightSourceCoord;

uniform sampler2DShadow texUnit;
uniform float bias;
uniform vec2 pixelDiff;

void main(void)
{
	vec4 shadowCoord = lightSourceCoord / lightSourceCoord.w;
	shadowCoord.z -= bias;

	float depth = 0;

	// 3x3 filtrering för snyggare skuggor
	for(int y = -1; y <= 1;y++){
		for(int x = -1; x <= 1;x++){
			depth += texture(texUnit, vec3(shadowCoord.x+x*pixelDiff.x, shadowCoord.y+y*pixelDiff.y, shadowCoord.z));
		}
	}
	depth /= 9;
		
	out_color = vec4(vec3(depth), 1);
}

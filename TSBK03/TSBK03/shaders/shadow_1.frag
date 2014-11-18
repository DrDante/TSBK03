#version 150

out vec4 out_color;
in vec4 lightSourceCoord;

uniform sampler2DShadow texUnit;
uniform float bias;

void main(void)
{
	vec4 shadowCoord = lightSourceCoord / lightSourceCoord.w;
	shadowCoord.z -= bias;

	float visibility = 1.0;
	float depth = texture(texUnit, vec3(shadowCoord));
		
	out_color = vec4(vec3(depth), 1);
}

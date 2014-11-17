#version 150

out vec4 out_color;
in vec4 lightSourceCoord;

uniform sampler2D texUnit;
uniform float bias;

void main(void)
{
	vec4 shadowCoord = lightSourceCoord / lightSourceCoord.w;

	float visibility = 1.0;
	float depth = textureProj(texUnit, shadowCoord);
	if(depth < (shadowCoord.z - bias)){
		visibility = 0.5;
	}
		
	out_color = vec4(vec3(visibility), 1);
}

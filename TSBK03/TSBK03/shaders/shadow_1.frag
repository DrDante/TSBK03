#version 150

out vec4 out_color;
in vec4 lightSourceCoord;

uniform sampler2D texUnit;

void main(void)
{
	vec4 shadowCoord = lightSourceCoord / lightSourceCoord.w;
	float visibility = 1.0;
	float depth = texture(texUnit, shadowCoord.st).z;
	vec4 color;
	if(depth < shadowCoord.z){
		color = vec4(1,0,0,1);
	} else {
		color = vec4(0,1,0,1);
	}
		
	out_color = vec4(vec3(depth), 1);
}

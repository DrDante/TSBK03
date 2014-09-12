#version 150

//in vec2 out_TexCoord;
in vec3 outNormal;
in vec3 outCamPos;
in vec3 outObjPos;
out vec4 out_Color;

//uniform sampler2D texUnit, texUnit2, texUnit3;

//in float pos;

//uniform bool skybox;
//uniform bool lambert;
//uniform bool multitex;
//uniform bool transparent;
// -------------------LIGHT SOURCE(S)-------------------
// Light sources
const int numberOfLightSources = 4;

uniform vec3 lightSourcesDirPosArr[numberOfLightSources];
uniform vec3 lightSourcesColorArr[numberOfLightSources];
uniform float specularExponent[numberOfLightSources];
uniform int isDirectional[numberOfLightSources];

vec3 r[numberOfLightSources];
vec3 s[numberOfLightSources];	// Incident light.
// -----------------------------------------------------
vec3 eye;	// Vector from the object to the camera.

float kdiff;
float kspec;
vec3 ambLight;		// Ambient light.
vec3 diffLight;		// Diffuse light.
vec3 specLight;		// Specular light.
vec3 totalLight;	// Total light.

// USCH
in vec3 tempLightDir;
// ----

void main(void)
{
	// Calculates the incident and reflected light rays for all light sources.
	for (int i = 0; i < numberOfLightSources; i++)
	{
		s[i] = normalize(lightSourcesDirPosArr[i] - (1 - isDirectional[i]) * outObjPos);
		r[i] = normalize(2 * outNormal * dot(normalize(s[i]), normalize(outNormal)) - s[i]);
	}
	// -----------------------------USCH-----------------------------
	s[0] = normalize(tempLightDir - (1 - isDirectional[0]) * outObjPos);
	r[0] = normalize(2 * outNormal * dot(normalize(s[0]), normalize(outNormal)) - s[0]);
	// --------------------------------------------------------------
	// Calculates the eye vector.
	eye = normalize(outCamPos-outObjPos);

	// Lighting according to the Phong model.
	kdiff = 3.5;
	kspec = 1.0;
	ambLight = kdiff * vec3(0.1, 0.1, 0.1);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Calculates diffuse light.
	for (int i = 0; i < numberOfLightSources; i++)
	{
		diffLight += kdiff * lightSourcesColorArr[i] * max(0.0, dot(s[i], normalize(outNormal)));
	}
	/*
	// -------------TILL SENARE-------------
	// Calculates specular light, if the surface is not Lambertian.
	if(!lambert)
	{
		for (int i = 0; i < numberOfLightSources; i++)
		{
			specLight += kspec * lightSourcesColorArr[i] * max(0.0, pow(dot(r[i], eye), specularExponent[i]));
		}
	}
	// -------------------------------------
	*/
	totalLight = vec3(0.0, 0.0, 0.0);
	// Adds the different light components to the total light.
	totalLight += ambLight;
	totalLight += diffLight;
	totalLight += specLight;

	vec4 tempTotalLight = vec4(totalLight, 1.0);
	/*
	// -------------TILL SENARE-------------
	if(!multitex)
	{
		out_Color = texture(texUnit, out_TexCoord) * tempTotalLight;
	}
	else
	{
		float zone1end = 10.0;
		float zone2start = 18.0;
		float zone2end = 25.0;
		float zone3start = 60.0;
		vec4 tempOutColor = vec4(0.0, 0.0, 0.0, 0.0);
		if(pos < zone1end)
			out_Color = texture(texUnit, out_TexCoord) * tempTotalLight;
		else if(pos >= zone1end && pos < zone2start)
		{
			tempOutColor += texture(texUnit, out_TexCoord) * (zone2start - pos) / (zone2start - zone1end);
			tempOutColor += texture(texUnit2, out_TexCoord) * (pos - zone1end) / (zone2start - zone1end);
			out_Color = tempOutColor * tempTotalLight;
		}
		else if(pos >= zone2start && pos < zone2end)
			out_Color = texture(texUnit2, out_TexCoord) * tempTotalLight;
		else if(pos >= zone2end && pos < zone3start)
		{
			tempOutColor += texture(texUnit2, out_TexCoord) * (zone3start - pos) / (zone3start - zone2end);
			tempOutColor += texture(texUnit3, out_TexCoord) * (pos - zone2end) / (zone3start - zone2end);
			out_Color = tempOutColor * tempTotalLight;
		}
		else
			out_Color = texture(texUnit3, out_TexCoord) * tempTotalLight;
	}
	// -------------------------------------
	*/
	out_Color = tempTotalLight;
}

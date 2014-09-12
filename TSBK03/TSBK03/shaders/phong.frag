#version 150

//in vec2 out_TexCoord;
in vec3 outNormal;
in vec3 outCamPos;
in vec3 outObjPos;
out vec4 out_Color;

//uniform sampler2D texUnit, texUnit2, texUnit3;

// -----Checklista p� saker vi beh�ver egna shaders till/titta vidare p�-----
//uniform bool skybox;
//uniform bool lambert;
//uniform bool multitex;
//uniform bool transparent;
// --------------------------------------------------------------------------
// ------------------------Ljusk�llor------------------------
const int numberOfLightSources = 4;		// �ndra h�r (manuellt) om m�ngden ljusk�llor �ndras.
										// Ska matcha NUM_LIGHTS i main.cpp.

uniform vec3 lightSourcesDirPosArr[numberOfLightSources];
uniform vec3 lightSourcesColorArr[numberOfLightSources];
uniform float specularExponent[numberOfLightSources];
uniform int isDirectional[numberOfLightSources];

vec3 r[numberOfLightSources];
vec3 s[numberOfLightSources];			// Infallande ljus.
// ----------------------------------------------------------
vec3 eye;			// Vektor fr�n objektet till kameran.

// Phong-modellen:
float kdiff;
float kspec;
vec3 ambLight;		// Ambient.
vec3 diffLight;		// Diffuse.
vec3 specLight;		// Specular.
vec3 totalLight;	// Totalt ljus.

// ------------------------------------------USCH------------------------------------------
in vec3 tempLightDir;
// ----------------------------------------------------------------------------------------

void main(void)
{
	// Infallande och reflekterat ljus ber�knas f�r alla ljusk�llor.
	for (int i = 0; i < numberOfLightSources; i++)
	{
		s[i] = normalize(lightSourcesDirPosArr[i] - (1 - isDirectional[i]) * outObjPos);
		r[i] = normalize(2 * outNormal * dot(normalize(s[i]), normalize(outNormal)) - s[i]);
	}
	// ------------------------------------------USCH------------------------------------------
	s[0] = normalize(tempLightDir - (1 - isDirectional[0]) * outObjPos);
	r[0] = normalize(2 * outNormal * dot(normalize(s[0]), normalize(outNormal)) - s[0]);
	// ----------------------------------------------------------------------------------------
	// eye-vektorn ber�knas.
	eye = normalize(outCamPos-outObjPos);

	// Ljus enligt Phong-modellen:
	kdiff = 3.5;
	kspec = 1.0;
	ambLight = kdiff * vec3(0.1, 0.1, 0.1);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Diffuse-ljus ber�knas.
	for (int i = 0; i < numberOfLightSources; i++)
	{
		diffLight += kdiff * lightSourcesColorArr[i] * max(0.0, dot(s[i], normalize(outNormal)));
	}
	/*
	// --------------B�r fixas s� sm�ningom (se "checklistan" ovan)--------------
	// Calculates specular light, if the surface is not Lambertian.
	if(!lambert)
	{
		for (int i = 0; i < numberOfLightSources; i++)
		{
			specLight += kspec * lightSourcesColorArr[i] * max(0.0, pow(dot(r[i], eye), specularExponent[i]));
		}
	}
	// --------------------------------------------------------------------------
	*/
	totalLight = vec3(0.0, 0.0, 0.0);
	// De olika ljuskomponenterna adderas till det totala ljuset.
	totalLight += ambLight;
	totalLight += diffLight;
	totalLight += specLight;

	vec4 tempTotalLight = vec4(totalLight, 1.0);

	out_Color = tempTotalLight;
}
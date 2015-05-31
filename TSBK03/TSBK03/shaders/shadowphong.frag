#version 150

out vec4 out_color;
in vec4 lightSourceCoord;

in vec3 outNormal;
in vec3 outCamPos;
in vec3 outObjPos;

uniform sampler2DShadow texUnit;
uniform float bias;
uniform vec2 pixelDiff;

uniform vec3 lPos;	// Ljuspositionen.

vec3 r;
vec3 s;				// Infallande ljus.
vec3 eye;			// Vektor från objektet till kameran.

int isDirectional;
vec3 lightSourceColor;
float specularExponent;

// Phong-modellen:
float kdiff;
float kspec;
vec3 ambLight;		// Ambient.
vec3 diffLight;		// Diffuse.
vec3 specLight;		// Specular.
vec3 totalLight;	// Totalt ljus.

float distance;
float cutoff;
float rangeComp;


void main(void)
{
	// --- Phong stuff ---
	// --- FIXA ---
	isDirectional = 0;
	lightSourceColor = vec3(1.0, 1.0, 1.0);
	specularExponent = 280.0;
	// ------------
	// Infallande och reflekterat ljus beräknas för alla ljuskällor.
	s = normalize(vec3(lPos.x, lPos.y, lPos.z) - (1 - isDirectional) * outObjPos);
	r = normalize(2 * outNormal * dot(normalize(s), normalize(outNormal)) - s);

	// eye-vektorn beräknas.
	eye = normalize(outCamPos - outObjPos);

	// Ljus enligt Phong-modellen:
	kdiff = 0.6;
	kspec = 1.0;
	ambLight = kdiff * vec3(0.1, 0.1, 0.1);
	diffLight = vec3(0.0, 0.0, 0.0);
	specLight = vec3(0.0, 0.0, 0.0);
	// Diffuse-ljus beräknas.
	diffLight += kdiff * lightSourceColor * max(0.0, dot(s, normalize(outNormal)));
	// Spekulärt ljus.
	specLight += kspec * lightSourceColor * max(0.0, pow(dot(r, eye), specularExponent));

	totalLight = vec3(0.0, 0.0, 0.0);
	// De olika ljuskomponenterna adderas till det totala ljuset.
	totalLight += ambLight;
	totalLight += diffLight;
	totalLight += specLight;

	// -------------------

	vec4 shadowCoord = lightSourceCoord / lightSourceCoord.w;
	shadowCoord.z -= bias;

	float depth = 0;

	// filter_side x filter_side filtrering för snyggare skuggor
	int filter_side = 5;
	for(int y = -(filter_side - 1)/2; y <= (filter_side - 1)/2; y++)
	{
		for(int x = -(filter_side - 1)/2; x <= (filter_side - 1)/2; x++)
		{
			float currTex = 0.0;
			if(shadowCoord.x + x*pixelDiff.x > 0.0 && shadowCoord.y + y*pixelDiff.y > 0.0 && shadowCoord.x + x*pixelDiff.x < 1.0 && shadowCoord.y + y*pixelDiff.y < 1.0)
			{
				currTex = texture(texUnit, vec3(shadowCoord.x + x*pixelDiff.x, shadowCoord.y + y*pixelDiff.y, shadowCoord.z));
				depth += currTex;
			}
		}
	}
	depth /= (filter_side*filter_side); // EGENTLIGEN dividerar man med för mycket i vissa fall, men det kommer inte märkas i praktiken.
	
	distance = length(outObjPos - lPos);
	int kappa = 10000; // Påverkar hur snabbt ljusintensitet avtar m.a.p. avstånd.
	rangeComp = kappa/(kappa + distance*distance);

	totalLight = totalLight * depth * rangeComp;
	out_color = vec4(totalLight, 1);
}

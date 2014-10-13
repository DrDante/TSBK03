#version 150

in vec3 in_Position;	// Vertex-positionen.
//in vec2 in_TexCoord;	// Texturkoordinat.
in vec3 in_Normal;		// Vertex-normal.

//out vec2 out_TexCoord;
out vec3 outNormal;
out vec3 outCamPos;
out vec3 outObjPos;

uniform mat4 MTWMatrix;	// Model-to-world-matris.
uniform mat4 WTVMatrix;	// World-to-view-matris (kamera).
uniform mat4 VTPMatrix;	// View-to-projection-matris (lins).
uniform vec3 camPos;	// Kamernapositionen.

// Och här trodde jag ZPR flyttade kameran, inte kaninen. Vettigt i en ljusuppgift. Suck...
// All kod med USCH i phongshadern är för att flytta ljuset med kaninen (WTV).
// Jag räknar med att vi kan slänga den här koden när vi har en egen kameraflyttningsfunktion.
uniform vec3 lightSourcesDirPosArr[4];
out vec3 tempLightDir;
// ----------------------------------------------------------------------------------------

// Nedanstående bör fixas.
/* mat3 normalTransformMatrix = transpose(inverse(mat3(WTVMatrix * MTWMatrix))); */
/* mat3 normalTransformMatrix = inverse(transpose(mat3(WTVMatrix * MTWMatrix))); */
mat3 normalTransformMatrix = mat3(WTVMatrix * MTWMatrix);

void main(void)
{
	gl_Position = VTPMatrix * WTVMatrix * MTWMatrix * vec4(in_Position, 1.0);
	//out_TexCoord = in_TexCoord;
	outNormal = normalTransformMatrix * in_Normal;
	outCamPos = camPos;
	outObjPos = vec3(MTWMatrix * vec4(in_Position, 1));
}

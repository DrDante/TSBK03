#version 150

in vec3 in_Position;	// Vertex position.
//in vec2 in_TexCoord;	// Texture coordinate.
in vec3 in_Normal;	// Vertex normal.

//out vec2 out_TexCoord;
out vec3 outNormal;
out vec3 outCamPos;
out vec3 outObjPos;

//out float pos;

uniform mat4 MTWMatrix;	// Model-to-world matrix.
uniform mat4 WTVMatrix;	// World-to-view matrix (camera).
uniform mat4 VTPMatrix;	// View-to-projection matrix (lens).
uniform vec3 camPos;	// Camera position.

// Och här trodde jag ZPR flyttade kameran, inte kaninen. Suck...
const int numberOfLightSources = 4;
uniform vec3 lightSourcesDirPosArr[numberOfLightSources];
out vec3 tempLightDir;
// --------------------------------------------------------------

mat3 normalTransformMatrix = transpose(inverse(mat3(WTVMatrix * MTWMatrix)));

void main(void)
{
	gl_Position = VTPMatrix * WTVMatrix * MTWMatrix * vec4(in_Position, 1.0);
	//out_TexCoord = in_TexCoord;
	outNormal = normalTransformMatrix * in_Normal;
	
	//vec3 tempPos = vec3(MTWMatrix * vec4(in_Position, 1.0));
	//pos = tempPos.y;

	vec3 temp = vec3(camPos[0], camPos[1], camPos[2]);
	outCamPos = temp;
	outObjPos = vec3(MTWMatrix * vec4(in_Position, 1));

	// -----------------------------USCH-----------------------------
	tempLightDir = mat3(WTVMatrix) * lightSourcesDirPosArr[0];
	// --------------------------------------------------------------
}

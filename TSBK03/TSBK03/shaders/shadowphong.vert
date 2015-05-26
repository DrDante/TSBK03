#version 150

in vec3 in_Position;
in vec3 in_Normal;		// Vertex-normal.

out vec4 lightSourceCoord;
out vec3 outNormal;
out vec3 outCamPos;
out vec3 outObjPos;

uniform mat4 MTWMatrix;	// Model-to-world-matris.
uniform mat4 WTVMatrix;	// World-to-view-matris (kamera).
uniform mat4 VTPMatrix;	// View-to-projection-matris (lins).
uniform mat4 textureMatrix;  // Scale and Bias matrix
uniform vec3 camPos;	// Kamernapositionen.

mat3 normalTransformMatrix = mat3(MTWMatrix);

void main(void)
{
	lightSourceCoord = textureMatrix * vec4(in_Position, 1.0);
	gl_Position = VTPMatrix * WTVMatrix * MTWMatrix * vec4(in_Position, 1.0);

	outObjPos = vec3(MTWMatrix * vec4(in_Position, 1));
	outNormal = normalTransformMatrix * in_Normal;
	outCamPos = camPos;
}
